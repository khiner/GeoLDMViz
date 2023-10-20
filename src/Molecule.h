#pragma once

#include <filesystem>

#include "Mesh/Primitive/Cylinder.h"
#include "Mesh/Primitive/Sphere.h"

#include "Scene.h"

namespace fs = std::filesystem;

struct Molecule {
    Molecule(const fs::path &xyz_file_path);
    ~Molecule();

    float GetAtomRadius(int atom_index) const;
    void SetAtomScale(float scale);
    void SetBondRadius(float scale);

    Mesh AtomMesh{Sphere{}}; // Single sphere mesh with an instance per atom.
    Mesh BondMesh{Cylinder{}}; // Single cylinder mesh with an instance per bond.

    fs::path XyzFilePath;

    std::vector<uint> AtomTypes;
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
    float AtomScale{0.5}, BondRadius{1};
    bool ShowBonds{true};
    bool AnimateChain{false};
    float AnimationSpeed{0.002};
    float AnimateTime{0};
};
