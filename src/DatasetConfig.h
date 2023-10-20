#pragma once

// Based on `qm9_with_h` in https://github.com/MinkaiXu/GeoLDM/blob/main/configs/datasets_config.py
// and https://github.com/MinkaiXu/GeoLDM/blob/main/qm9/bond_analyze.py

#include <string>
#include <unordered_map>
#include <vector>

#include <glm/vec4.hpp>

using uint = unsigned int;

static const std::unordered_map<std::string, std::unordered_map<std::string, uint>> Bonds1 = {
    {"H", {{"H", 74}, {"C", 109}, {"N", 101}, {"O", 96}, {"F", 92}, {"B", 119}, {"Si", 148}, {"P", 144}, {"As", 152}, {"S", 134}, {"Cl", 127}, {"Br", 141}, {"I", 161}}},
    {"C", {{"H", 109}, {"C", 154}, {"N", 147}, {"O", 143}, {"F", 135}, {"Si", 185}, {"P", 184}, {"S", 182}, {"Cl", 177}, {"Br", 194}, {"I", 214}}},
    {"N", {{"H", 101}, {"C", 147}, {"N", 145}, {"O", 140}, {"F", 136}, {"Cl", 175}, {"Br", 214}, {"S", 168}, {"I", 222}, {"P", 177}}},
    {"O", {{"H", 96}, {"C", 143}, {"N", 140}, {"O", 148}, {"F", 142}, {"Br", 172}, {"S", 151}, {"P", 163}, {"Si", 163}, {"Cl", 164}, {"I", 194}}},
    {"F", {{"H", 92}, {"C", 135}, {"N", 136}, {"O", 142}, {"F", 142}, {"S", 158}, {"Si", 160}, {"Cl", 166}, {"Br", 178}, {"P", 156}, {"I", 187}}},
    {"B", {{"H", 119}, {"Cl", 175}}},
    {"Si", {{"Si", 233}, {"H", 148}, {"C", 185}, {"O", 163}, {"S", 200}, {"F", 160}, {"Cl", 202}, {"Br", 215}, {"I", 243}}},
    {"Cl", {{"Cl", 199}, {"H", 127}, {"C", 177}, {"N", 175}, {"O", 164}, {"P", 203}, {"S", 207}, {"B", 175}, {"Si", 202}, {"F", 166}, {"Br", 214}}},
    {"S", {{"H", 134}, {"C", 182}, {"N", 168}, {"O", 151}, {"S", 204}, {"F", 158}, {"Cl", 207}, {"Br", 225}, {"Si", 200}, {"P", 210}, {"I", 234}}},
    {"Br", {{"Br", 228}, {"H", 141}, {"C", 194}, {"O", 172}, {"N", 214}, {"Si", 215}, {"S", 225}, {"F", 178}, {"Cl", 214}, {"P", 222}}},
    {"P", {{"P", 221}, {"H", 144}, {"C", 184}, {"O", 163}, {"Cl", 203}, {"S", 210}, {"F", 156}, {"N", 177}, {"Br", 222}}},
    {"I", {{"H", 161}, {"C", 214}, {"Si", 243}, {"N", 222}, {"O", 194}, {"S", 234}, {"F", 187}, {"I", 266}}},
    {"As", {{"H", 152}}}};

static const std::unordered_map<std::string, std::unordered_map<std::string, uint>> Bonds2 = {
    {"C", {{"C", 134}, {"N", 129}, {"O", 120}, {"S", 160}}},
    {"N", {{"C", 129}, {"N", 125}, {"O", 121}}},
    {"O", {{"C", 120}, {"N", 121}, {"O", 121}, {"P", 150}}},
    {"P", {{"O", 150}, {"S", 186}}},
    {"S", {{"P", 186}}}};

static const std::unordered_map<std::string, std::unordered_map<std::string, uint>> Bonds3 = {
    {"C", {{"C", 120}, {"N", 116}, {"O", 113}}},
    {"N", {{"C", 116}, {"N", 110}}},
    {"O", {{"C", 113}}}};

inline static int GetBondOrder(const std::string &atom1, const std::string &atom2, double distance, bool check_exists = false) {
    static const float Margin1 = 10, Margin2 = 5, Margin3 = 3;
    distance *= 100.0;

    // Check exists for large molecules where some atom pairs do not have a typical bond length.
    if (check_exists) {
        if (Bonds1.find(atom1) == Bonds1.end()) return 0;
        if (Bonds1.at(atom1).find(atom2) == Bonds1.at(atom1).end()) return 0;
    }

    if (distance < Bonds1.at(atom1).at(atom2) + Margin1) {
        // Check if atoms are in `Bonds2`.
        if (Bonds2.find(atom1) != Bonds2.end() && Bonds2.at(atom1).find(atom2) != Bonds2.at(atom1).end()) {
            const double thr_bond2 = Bonds2.at(atom1).at(atom2) + Margin2;
            if (distance < thr_bond2) {
                if (Bonds3.find(atom1) != Bonds3.end() && Bonds3.at(atom1).find(atom2) != Bonds3.at(atom1).end()) {
                    const double thr_bond3 = Bonds3.at(atom1).at(atom2) + Margin3;
                    if (distance < thr_bond3) return 3; // Triple
                }
                return 2; // Double
            }
        }
        return 1; // Single
    }
    return 0; // No bond
}

struct QM9WithH {
    std::string Name = "qm9";
    std::unordered_map<std::string, uint> AtomEncoder = {{"H", 0}, {"C", 1}, {"N", 2}, {"O", 3}, {"F", 4}};
    std::vector<std::string> AtomDecoder = {"H", "C", "N", "O", "F"};
    std::vector<glm::vec4> ColorForAtom = {
        glm::vec4{0.9f, 0.9f, 1.0f, 0.7f}, // H: Light blue tint with some transparency
        {0.2f, 0.2f, 0.2f, 1.0f}, // C: Dark Gray
        {0.0f, 0.0f, 1.0f, 1.0f}, // N: Blue
        {1.0f, 0.0f, 0.0f, 1.0f}, // O: Red
        {0.0f, 1.0f, 0.0f, 1.0f} // F: Green
    };
    std::vector<float> RadiusForAtom = {0.46f, 0.77f, 0.77f, 0.77f, 0.77f};
};
