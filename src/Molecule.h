#pragma once

#include <filesystem>

#include "Scene.h"

namespace fs = std::filesystem;

struct Molecule {
    Molecule(const fs::path &xyz_file_path, ::Scene *);

    std::pair<glm::vec3, glm::vec3> ComputeBounds() const {
        static const float min_float = std::numeric_limits<float>::lowest();
        static const float max_float = std::numeric_limits<float>::max();

        glm::vec3 min(max_float), max(min_float);
        for (const auto &mesh : AtomMeshes) {
            auto [mesh_bounds_min, mesh_bounds_max] = mesh.Triangles.ComputeBounds();
            min.x = std::min(min.x, mesh_bounds_min[0]);
            min.y = std::min(min.y, mesh_bounds_min[1]);
            min.z = std::min(min.z, mesh_bounds_min[2]);
            max.x = std::max(max.x, mesh_bounds_max[0]);
            max.y = std::max(max.y, mesh_bounds_max[1]);
            max.z = std::max(max.z, mesh_bounds_max[2]);
        }

        return {min, max};
    }

    void AddToScene();
    void RemoveFromScene();

    std::vector<Mesh> AtomMeshes; // Sphere mesh for each atom.

    ::Scene *Scene;
};

struct MoleculeChain {
    MoleculeChain(const fs::path &xyz_files_path, ::Scene *);

    void RenderConfig();

    std::vector<Molecule> Molecules;
    ::Scene *Scene;


private:
    void SetMoleculeIndex(int index);

    int MoleculeIndex{0};
};
