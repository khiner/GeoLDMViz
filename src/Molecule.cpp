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
    const uint num_atoms = std::stoi(line);
    // Second line is empty.
    std::getline(xyz_file, line);

    AtomMesh.Generate();
    AtomMesh.ClearInstances();
    AtomTypes.clear();
    int atom_index = 0;
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

    if (AtomMesh.NumInstances() != num_atoms) {
        std::cerr << "Expected " << num_atoms << " atoms, but found " << AtomMesh.NumInstances() << std::endl;
        return;
    }
}
Molecule::~Molecule() {
    AtomMesh.Delete();
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
    for (auto &molecule : Molecules) Scene->RemoveMesh(&molecule.AtomMesh);
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

    if (SliderFloat("Atom scale", &AtomScale, .01f, 10.f, "%.3f", ImGuiSliderFlags_Logarithmic)) {
        Molecules[MoleculeIndex].SetAtomScale(AtomScale);
    }

    if (Molecules.size() > 1) {
        int new_molecule_index = MoleculeIndex;
        if (SliderInt("Molecule", &new_molecule_index, 0, Molecules.size() - 1)) {
            SetMoleculeIndex(new_molecule_index);
        }
    }
}

void MoleculeChain::SetMoleculeIndex(int index) {
    if (index < 0 || index >= int(Molecules.size())) return;

    Scene->RemoveMesh(&Molecules[MoleculeIndex].AtomMesh);
    MoleculeIndex = index;
    auto &molecule = Molecules[MoleculeIndex];
    molecule.SetAtomScale(AtomScale);
    auto [bounds_min, bounds_max] = molecule.AtomMesh.ComputeBounds();
    Scene->AddMesh(&molecule.AtomMesh);
    Scene->SetCameraDistance(glm::distance(bounds_min, bounds_max) * 2);
}
