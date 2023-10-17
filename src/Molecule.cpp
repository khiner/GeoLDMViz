#include "Molecule.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include "Mesh/Primitive/Sphere.h"

Molecule::Molecule(const fs::path &xyz_file_path, ::Scene *scene) : Scene(scene) {
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
    while (std::getline(xyz_file, line)) {
        std::istringstream iss(line);
        std::string atom_name;
        glm::vec3 position;
        iss >> atom_name >> position.x >> position.y >> position.z;
        AtomMeshes.emplace_back(Sphere{0.5f});
        AtomMeshes.back().SetPosition(position);
        // AtomMeshes.back().SetColor({1, 1, 1, 1});
    }
    if (AtomMeshes.size() != num_atoms) {
        std::cerr << "Expected " << num_atoms << " atoms, but found " << AtomMeshes.size() << std::endl;
        return;
    }

    auto [bounds_min, bounds_max] = ComputeBounds();
    for (auto &mesh : AtomMeshes) {
        Scene->AddMesh(&mesh);
    }
    Scene->SetCameraDistance(glm::distance(bounds_min, bounds_max) * 2);
}
