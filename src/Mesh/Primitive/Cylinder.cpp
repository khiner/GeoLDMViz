#include "Cylinder.h"

Cylinder::Cylinder(float radius, float height, uint slices) : Geometry() {
    // Top cap.
    for (uint i = 0; i < slices; i++) {
        const float __a = 2.f * (float(i) / slices);
        Vertices.push_back({radius * __cospif(__a), height / 2, radius * __sinpif(__a)});
    }
    Vertices.push_back({0, height / 2, 0}); // Center.

    // Bottom cap.
    for (uint i = 0; i < slices; i++) {
        const float __a = 2.f * (float(i) / slices);
        Vertices.push_back({radius * __cospif(__a), -height / 2, radius * __sinpif(__a)});
    }
    Vertices.push_back({0, -height / 2, 0}); // Center.

    // Top/bottom caps.
    for (uint i = 0; i < slices; i++) Indices.insert(Indices.end(), {i, (i + 1) % slices, slices});
    const uint b_offset = slices + 1;
    for (uint i = 0; i < slices; i++) Indices.insert(Indices.end(), {b_offset + i, b_offset + ((i + 1) % slices), b_offset + slices});

    // Body.
    for (uint i = 0; i < slices; i++) {
        const uint top1 = i, top2 = (i + 1) % slices, bottom1 = b_offset + i, bottom2 = b_offset + ((i + 1) % slices);
        Indices.insert(Indices.end(), {top1, top2, bottom1, bottom1, top2, bottom2});
    }

    for (const auto &vertex : Vertices) Normals.push_back(glm::normalize(glm::vec3{vertex.x, 0, vertex.z}));
}
