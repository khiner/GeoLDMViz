#include "Molecule.h"

#include <fstream>
#include <iostream>
#include <sstream>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"

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
        for (uint j = 0; j < num_atoms; j++) {
            const auto p1 = AtomMesh.GetPosition(i);
            const auto p2 = AtomMesh.GetPosition(j);
            const auto dist = glm::distance(p1, p2);
            const auto atom1 = DatasetConfig.AtomDecoder[AtomTypes[i]];
            const auto atom2 = DatasetConfig.AtomDecoder[AtomTypes[j]];
            const auto s = std::minmax(AtomTypes[i], AtomTypes[j]);
            const auto pair = std::make_pair(DatasetConfig.AtomDecoder.at(s.first), DatasetConfig.AtomDecoder.at(s.second));
            const auto draw_edge_int = GetBondOrder(atom1, atom2, dist);
            const float line_width = (3.f - 2.f) * 2 * 2;
            const bool draw_edge = draw_edge_int > 0;
            if (draw_edge) {
                const float linewidth_factor = draw_edge_int == 4 ? 1.5f : 1.f;
                BondMesh.AddInstance();
                const auto midpoint = (p1 + p2) / 2.0f;
                const auto direction = glm::normalize(p2 - p1);
                const auto distance = glm::distance(p1, p2);

                // todo not quite right. Rotation is wrong.
                const glm::mat4 translation = glm::translate(glm::mat4(1.0f), midpoint);
                const glm::quat rotation = glm::quatLookAt(direction, Up);
                const glm::mat4 scale = glm::scale(Identity, glm::vec3(1.0f, distance, 1.0f));

                // const glm::mat4 translation = glm::translate(glm::mat4(1.0f), midpoint);
                // const glm::mat4 scale = glm::scale(Identity, glm::vec3(1.0f, distance, 1.0f));
                // const glm::vec3 old_up = {0.0f, 1.0f, 0.0f}; // The original 'up' direction of the cylinder
                // const glm::vec3 axis = glm::cross(old_up, direction); // Compute the axis of rotation
                // const float angle = glm::acos(glm::dot(old_up, direction)); // Compute the angle of rotation
                // const glm::quat rotation = glm::angleAxis(angle, axis); // Create the quaternion

                // Now use this quaternion in your transformation
                const glm::mat4 bond_transform = translation * glm::mat4_cast(rotation) * scale;
                BondMesh.SetTransform(bond_index, bond_transform);
                BondMesh.SetColor(bond_index, {1, 1, 1, 1});
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

        Molecules.reserve(paths.size());
        for (const auto &path : paths) {
            Molecules.emplace_back(path);
        }

        if (Molecules.empty()) {
            std::cerr << "No .txt files found in directory: " << xyz_files_path << std::endl;
            return;
        }
    }

    SetMoleculeIndex(0);
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

    Checkbox("Show bonds", &ShowBonds);
    Checkbox("Animate chain", &AnimateChain);
    SliderFloat("Animation speed", &AnimationSpeed, 0.00001f, 0.01f);

    if (SliderFloat("Atom scale", &AtomScale, .01f, 10.f, "%.3f", ImGuiSliderFlags_Logarithmic)) {
        Molecules[MoleculeIndex].SetAtomScale(AtomScale);
    }

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
    auto [bounds_min, bounds_max] = molecule.AtomMesh.ComputeBounds();
    Scene->AddMesh(&molecule.AtomMesh);
    // Scene->AddMesh(&molecule.BondMesh);
    Scene->SetCameraDistance(glm::distance(bounds_min, bounds_max) * 2);
}
