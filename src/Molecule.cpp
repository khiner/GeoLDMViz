#include "Molecule.h"

#include <fstream>
#include <iostream>
#include <sstream>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"

#include "DatasetConfig.h"
#include "Mesh/Primitive/Sphere.h"

Molecule::Molecule(const fs::path &xyz_file_path, ::Scene *scene) : Scene(scene) {
    std::ifstream xyz_file(xyz_file_path);
    if (!xyz_file.is_open()) {
        std::cerr << "Failed to open " << xyz_file_path << std::endl;
        return;
    }

    static const QM9WithH DatasetConfig;

    std::string line;
    // First line has the number of molecules.
    std::getline(xyz_file, line);
    const uint num_atoms = std::stoi(line);
    // Second line is empty.
    std::getline(xyz_file, line);
    while (std::getline(xyz_file, line)) {
        std::istringstream iss(line);
        std::string atom_name; // 'H', 'C', 'N', 'O', 'F'
        glm::vec3 position;
        iss >> atom_name >> position.x >> position.y >> position.z;

        const uint atom_type = DatasetConfig.AtomEncoder.at(atom_name);
        // Hydrogen, Carbon, Nitrogen, Oxygen, Flourine
        // area_dic = 1500 * radius_dic ** 2
        // # areas_dic = sizes_dic * sizes_dic * 3.1416
        // areas = area_dic[atom_type]
        AtomMeshes.emplace_back(Sphere{DatasetConfig.RadiusForAtom[atom_type]});
        AtomMeshes.back().SetPosition(position);
        AtomMeshes.back().SetColor(DatasetConfig.ColorForAtom.at(atom_type));
    }

    if (AtomMeshes.size() != num_atoms) {
        std::cerr << "Expected " << num_atoms << " atoms, but found " << AtomMeshes.size() << std::endl;
        return;
    }
}

void Molecule::AddToScene() {
    auto [bounds_min, bounds_max] = ComputeBounds();
    for (auto &mesh : AtomMeshes) Scene->AddMesh(&mesh);
    Scene->SetCameraDistance(glm::distance(bounds_min, bounds_max) * 2);
}

void Molecule::RemoveFromScene() {
    for (auto &mesh : AtomMeshes) Scene->RemoveMesh(&mesh);
}

MoleculeChain::MoleculeChain(const fs::path &xyz_files_path, ::Scene *scene) : Scene(scene) {
    if (!fs::is_directory(xyz_files_path)) {
        Molecules.emplace_back(xyz_files_path, Scene);
    } else {
        for (const auto &entry : fs::directory_iterator(xyz_files_path)) {
            const auto &path = entry.path();
            if (path.extension() == ".txt") Molecules.emplace_back(path, Scene);
        }

        if (Molecules.empty()) {
            std::cerr << "No .txt files found in directory: " << xyz_files_path << std::endl;
            return;
        }
    }

    Molecules[0].AddToScene();
}

using namespace ImGui;

void MoleculeChain::RenderConfig() {
    if (Molecules.empty()) {
        TextUnformatted("No molecules loaded.");
        return;
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

    Molecules[MoleculeIndex].RemoveFromScene();
    MoleculeIndex = index;
    Molecules[MoleculeIndex].AddToScene();
}
