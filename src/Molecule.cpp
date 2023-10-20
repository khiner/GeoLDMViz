#include "Molecule.h"

#include <fstream>
#include <iostream>
#include <sstream>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include <glm/gtx/matrix_decompose.hpp>

#include "DatasetConfig.h"
#include "Mesh/Primitive/Sphere.h"

static const QM9WithH DatasetConfig;

Molecule::Molecule(const fs::path &xyz_file_path) : XyzFilePath(xyz_file_path) {
    std::ifstream xyz_file(xyz_file_path);
    if (!xyz_file.is_open()) {
        std::cerr << "Failed to open " << xyz_file_path << std::endl;
        return;
    }

    std::string line;
    // First line has the number of molecules.
    std::getline(xyz_file, line);
    const uint expected_num_atoms = std::stoi(line);
    // Second line is empty.
    std::getline(xyz_file, line);

    AtomMesh.Generate();
    AtomMesh.ClearInstances();
    AtomTypes.clear();
    uint atom_index = 0;
    while (std::getline(xyz_file, line)) {
        std::istringstream iss(line);
        std::string atom_name; // 'H', 'C', 'N', 'O', 'F'
        glm::vec3 position;
        iss >> atom_name >> position.x >> position.y >> position.z;

        const uint atom_type = DatasetConfig.AtomEncoder.at(atom_name);
        AtomTypes.emplace_back(atom_type);
        AtomMesh.AddInstance();
        AtomMesh.SetPosition(atom_index, position);
        AtomMesh.SetScale(atom_index, GetAtomRadius(atom_index));
        AtomMesh.SetColor(atom_index, DatasetConfig.ColorForAtom.at(atom_type));
        atom_index++;
    }

    const uint num_atoms = AtomMesh.NumInstances();
    if (num_atoms != expected_num_atoms) {
        std::cerr << "Expected " << expected_num_atoms << " atoms, but found " << AtomMesh.NumInstances() << std::endl;
    }

    BondMesh.Generate();
    BondMesh.ClearInstances();
    uint bond_index = 0;
    for (uint i = 0; i < num_atoms; i++) {
        for (uint j = 0; j < i; j++) {
            const auto p1 = AtomMesh.GetPosition(i);
            const auto p2 = AtomMesh.GetPosition(j);
            const auto dist = glm::distance(p1, p2);
            const auto atom1 = DatasetConfig.AtomDecoder[AtomTypes[i]];
            const auto atom2 = DatasetConfig.AtomDecoder[AtomTypes[j]];
            const auto s = std::minmax(AtomTypes[i], AtomTypes[j]);
            const auto pair = std::make_pair(DatasetConfig.AtomDecoder.at(s.first), DatasetConfig.AtomDecoder.at(s.second));
            const auto draw_edge_int = GetBondOrder(atom1, atom2, dist);
            if (draw_edge_int > 0) {
                const auto midpoint = (p1 + p2) / 2.0f;
                const auto direction = glm::normalize(p2 - p1);
                const auto distance = glm::distance(p1, p2);
                // `quatLookAt` assumes the forward direction is -Z, so we need to rotate the result by 90 degrees.
                const glm::quat rotation = glm::quatLookAt(direction, Up) * glm::angleAxis(float(M_PI / 2), glm::vec3{1, 0, 0});
                const glm::mat4 transform = glm::scale(glm::translate(Identity, midpoint) * glm::mat4_cast(rotation), {1, distance, 1});
                BondMesh.AddInstance();
                BondMesh.SetTransform(bond_index, transform);
                bond_index++;
            }
        }
    }
}

Molecule::~Molecule() {
    AtomMesh.Delete();
    BondMesh.Delete();
}

float Molecule::GetAtomRadius(int atom_index) const { return DatasetConfig.RadiusForAtom[AtomTypes[atom_index]]; }

void Molecule::SetAtomScale(float scale) {
    for (uint atom_index = 0; atom_index < AtomMesh.NumInstances(); atom_index++) {
        AtomMesh.SetScale(atom_index, GetAtomRadius(atom_index) * scale);
    }
}

void Molecule::SetBondRadius(float radius) {
    for (uint bond_index = 0; bond_index < BondMesh.NumInstances(); bond_index++) {
        // Each bond may have arbitrary translation, rotation and y-scaling applied.
        glm::vec3 scale, translation, skew;
        glm::vec4 perspective;
        glm::quat rotation;
        if (glm::decompose(BondMesh.GetTransform(bond_index), scale, rotation, translation, skew, perspective)) {
            scale.x = scale.z = radius;
            BondMesh.SetTransform(bond_index, glm::scale(glm::translate(Identity, translation) * glm::mat4_cast(rotation), scale));
        }
    }
}

MoleculeChain::MoleculeChain(const fs::path &xyz_files_path, ::Scene *scene) : Scene(scene) {
    if (!fs::is_directory(xyz_files_path)) {
        Molecules.emplace_back(xyz_files_path);
    } else {
        std::vector<fs::path> paths;
        for (const auto &entry : fs::directory_iterator(xyz_files_path)) {
            const auto &path = entry.path();
            if (path.extension() == ".txt") paths.push_back(path);
        }
        std::sort(paths.begin(), paths.end());

        // `Molecule`'s move semantics don't currently work with OpenGL state well, so use `reserve` to avoid reallocations.
        Molecules.reserve(paths.size());
        for (const auto &path : paths) Molecules.emplace_back(path);

        if (Molecules.empty()) {
            std::cerr << "No .txt files found in directory: " << xyz_files_path << std::endl;
            return;
        }
    }

    SetMoleculeIndex(Molecules.size() - 1); // Default to the final molecule in the chain.
}

MoleculeChain::~MoleculeChain() {
    for (auto &molecule : Molecules) {
        Scene->RemoveMesh(&molecule.AtomMesh);
        Scene->RemoveMesh(&molecule.BondMesh);
    }
}

using namespace ImGui;

void MoleculeChain::RenderConfig() {
    if (Molecules.empty()) {
        TextUnformatted("No molecules loaded.");
        return;
    }

    MoleculeIndex = std::clamp(MoleculeIndex, 0, int(Molecules.size() - 1));

    std::string file_name = Molecules[MoleculeIndex].XyzFilePath.filename().string();
    Text("Current molecule:\n\t%s", file_name.c_str());

    if (Checkbox("Show bonds", &ShowBonds)) SetMoleculeIndex(MoleculeIndex);
    if (!ShowBonds) BeginDisabled();
    if (SliderFloat("Bond radius", &BondRadius, .01f, 4.f, "%.3f", ImGuiSliderFlags_Logarithmic)) {
        Molecules[MoleculeIndex].SetBondRadius(BondRadius);
    }
    if (!ShowBonds) EndDisabled();
    if (SliderFloat("Atom scale", &AtomScale, .01f, 4.f, "%.3f", ImGuiSliderFlags_Logarithmic)) {
        Molecules[MoleculeIndex].SetAtomScale(AtomScale);
    }

    Checkbox("Animate chain", &AnimateChain);
    SliderFloat("Animation speed", &AnimationSpeed, 0.00001f, 0.01f);

    if (Molecules.size() > 1) {
        int new_molecule_index = MoleculeIndex;
        if (SliderInt("Molecule", &new_molecule_index, 0, Molecules.size() - 1)) {
            AnimateChain = false;
            SetMoleculeIndex(new_molecule_index);
        }
    }

    if (AnimateChain) {
        AnimateTime += AnimationSpeed;
        if (AnimateTime >= 1) AnimateTime = 0;
        SetMoleculeIndex(uint(AnimateTime * Molecules.size()) % Molecules.size());
    }
}

void MoleculeChain::SetMoleculeIndex(int index) {
    if (index < 0 || index >= int(Molecules.size())) return;

    Scene->RemoveMesh(&Molecules[MoleculeIndex].AtomMesh);
    Scene->RemoveMesh(&Molecules[MoleculeIndex].BondMesh);
    MoleculeIndex = index;
    auto &molecule = Molecules[MoleculeIndex];
    molecule.SetAtomScale(AtomScale);
    molecule.SetBondRadius(BondRadius);
    auto [bounds_min, bounds_max] = molecule.AtomMesh.ComputeBounds();
    Scene->AddMesh(&molecule.AtomMesh);
    if (ShowBonds) Scene->AddMesh(&molecule.BondMesh);
    Scene->SetCameraDistance(glm::distance(bounds_min, bounds_max) * 2);
}
