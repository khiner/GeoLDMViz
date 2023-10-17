#pragma once

// Based on `qm9_with_h` in https://github.com/MinkaiXu/GeoLDM/blob/main/configs/datasets_config.py

#include <string>
#include <unordered_map>
#include <vector>

#include <glm/vec4.hpp>

using uint = unsigned int;

struct QM9WithH {
    std::string Name = "qm9";
    std::unordered_map<std::string, uint> AtomEncoder = {{"H", 0}, {"C", 1}, {"N", 2}, {"O", 3}, {"F", 4}};
    std::vector<std::string> AtomDecodder = {"H", "C", "N", "O", "F"};
    std::unordered_map<uint, uint> NumNodes = {
        {22, 3393}, {17, 13025}, {23, 4848}, {21, 9970}, {19, 13832}, {20, 9482}, {16, 10644}, {13, 3060}, {15, 7796}, {25, 1506}, {18, 13364}, {12, 1689}, {11, 807}, {24, 539}, {14, 5136}, {26, 48}, {7, 16}, {10, 362}, {8, 49}, {9, 124}, {27, 266}, {4, 4}, {29, 25}, {6, 9}, {5, 5}, {3, 1}};
    int MaxNumNodes = 29;
    std::unordered_map<uint, uint> AtomTypes = {{1, 635559}, {2, 101476}, {0, 923537}, {3, 140202}, {4, 2323}};
    std::vector<int> Distances = {
        903054, 307308, 111994, 57474, 40384, 29170, 47152, 414344, 2202212, 573726,
        1490786, 2970978, 756818, 969276, 489242, 1265402, 4587994, 3187130, 2454868, 2647422,
        2098884,
        2001974, 1625206, 1754172, 1620830, 1710042, 2133746, 1852492, 1415318, 1421064, 1223156,
        1322256,
        1380656, 1239244, 1084358, 981076, 896904, 762008, 659298, 604676, 523580, 437464, 413974,
        352372,
        291886, 271948, 231328, 188484, 160026, 136322, 117850, 103546, 87192, 76562, 61840,
        49666, 43100,
        33876, 26686, 22402, 18358, 15518, 13600, 12128, 9480, 7458, 5088, 4726, 3696, 3362, 3396,
        2484,
        1988, 1490, 984, 734, 600, 456, 482, 378, 362, 168, 124, 94, 88, 52, 44, 40, 18, 16, 8, 6,
        2,
        0, 0, 0, 0,
        0,
        0, 0};
    std::vector<glm::vec4> ColorForAtom = {
        glm::vec4{0.9f, 0.9f, 1.0f, 0.7f}, // H: Light blue tint with some transparency
        {0.2f, 0.2f, 0.2f, 1.0f}, // C: Dark Gray
        {0.0f, 0.0f, 1.0f, 1.0f}, // N: Blue
        {1.0f, 0.0f, 0.0f, 1.0f}, // O: Red
        {0.0f, 1.0f, 0.0f, 1.0f} // F: Green
    };

    std::vector<float> RadiusForAtom = {0.46f, 0.77f, 0.77f, 0.77f, 0.77f};
};
