#pragma once

#include <filesystem>

#include "Mesh/Primitive/Sphere.h"
#include "Scene.h"

namespace fs = std::filesystem;

struct Molecule {
    Molecule(const fs::path &xyz_file_path, ::Scene *);
    ~Molecule();

    void AddToScene();
    void RemoveFromScene();

    Mesh AtomMesh{Sphere{}}; // Single sphere mesh with an instance per atom.

    ::Scene *Scene;
    fs::path XyzFilePath;
};

struct MoleculeChain {
    MoleculeChain(const fs::path &xyz_files_path, ::Scene *);
    ~MoleculeChain();

    void RenderConfig();

    std::vector<Molecule> Molecules;
    ::Scene *Scene;

private:
    void SetMoleculeIndex(int index);

    int MoleculeIndex{0};
};
