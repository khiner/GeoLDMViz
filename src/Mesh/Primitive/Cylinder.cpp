#include "Cylinder.h"

Cylinder::Cylinder(float radius, float height, int slices) : Geometry() {
    // Top cap
    for (int i = 0; i < slices; i++) {
        float theta = (float)i / slices * 2.0f * M_PI;
        Vertices.push_back({radius * cos(theta), height / 2, radius * sin(theta)});
    }
    Vertices.push_back({0, height / 2, 0}); // Center vertex

    // Bottom cap
    for (int i = 0; i < slices; i++) {
        float theta = (float)i / slices * 2.0f * M_PI;
        Vertices.push_back({radius * cos(theta), -height / 2, radius * sin(theta)});
    }
    Vertices.push_back({0, -height / 2, 0}); // Center vertex

    // Top Cap Indices
    for (int i = 0; i < slices; i++) {
        Indices.push_back(i);
        Indices.push_back((i + 1) % slices);
        Indices.push_back(slices); // Center vertex
    }

    // Bottom Cap Indices
    int bottom_offset = slices + 1;
    for (int i = 0; i < slices; i++) {
        Indices.push_back(bottom_offset + i);
        Indices.push_back(bottom_offset + ((i + 1) % slices));
        Indices.push_back(bottom_offset + slices); // Center vertex
    }

    // Body
    for (int i = 0; i < slices; i++) {
        uint top1 = i;
        uint top2 = (i + 1) % slices;
        uint bottom1 = bottom_offset + i;
        uint bottom2 = bottom_offset + ((i + 1) % slices);
        Indices.push_back(top1);
        Indices.push_back(top2);
        Indices.push_back(bottom1);

        Indices.push_back(bottom1);
        Indices.push_back(top2);
        Indices.push_back(bottom2);
    }

    for (const auto &vertex : Vertices) {
        Normals.push_back(glm::normalize(glm::vec3{vertex.x, 0, vertex.z}));
    }
}
