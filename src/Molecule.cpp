#include "Molecule.h"

#include <fstream>
#include <iostream>
#include <sstream>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"

#include "DatasetConfig.h"
#include "Mesh/Primitive/Sphere.h"

Molecule::Molecule(const fs::path &xyz_file_path, ::Scene *scene) : Scene(scene), XyzFilePath(xyz_file_path) {
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

    AtomMesh.Generate();
    AtomMesh.ClearInstances();
    int atom_index = 0;
    while (std::getline(xyz_file, line)) {
        std::istringstream iss(line);
        std::string atom_name; // 'H', 'C', 'N', 'O', 'F'
        glm::vec3 position;
        iss >> atom_name >> position.x >> position.y >> position.z;

        const uint atom_type = DatasetConfig.AtomEncoder.at(atom_name);
        AtomMesh.AddInstance();
        AtomMesh.SetPosition(atom_index, position);
        AtomMesh.SetScale(atom_index, DatasetConfig.RadiusForAtom[atom_type]);
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

void Molecule::AddToScene() {
    auto [bounds_min, bounds_max] = AtomMesh.ComputeBounds();
    Scene->AddMesh(&AtomMesh);
    Scene->SetCameraDistance(glm::distance(bounds_min, bounds_max) * 2);
}

void Molecule::RemoveFromScene() {
    Scene->RemoveMesh(&AtomMesh);
}

MoleculeChain::MoleculeChain(const fs::path &xyz_files_path, ::Scene *scene) : Scene(scene) {
    if (!fs::is_directory(xyz_files_path)) {
        Molecules.emplace_back(xyz_files_path, Scene);
    } else {
        std::vector<fs::path> paths;
        for (const auto &entry : fs::directory_iterator(xyz_files_path)) {
            const auto &path = entry.path();
            if (path.extension() == ".txt") paths.push_back(path);
        }
        std::sort(paths.begin(), paths.end());

        Molecules.reserve(paths.size());
        for (const auto &path : paths) {
            Molecules.emplace_back(path, Scene);
        }

        if (Molecules.empty()) {
            std::cerr << "No .txt files found in directory: " << xyz_files_path << std::endl;
            return;
        }
    }

    Molecules[MoleculeIndex].AddToScene();
}

MoleculeChain::~MoleculeChain() {
    for (auto &molecule : Molecules) molecule.RemoveFromScene();
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
