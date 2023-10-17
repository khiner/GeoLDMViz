#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

using uint = unsigned int;

namespace fs = std::filesystem;

struct MeshBuffers {
    MeshBuffers() {}
    virtual ~MeshBuffers() = default;

    void Clear() {
        Vertices.clear();
        Normals.clear();
        Indices.clear();
        Dirty = true;
    }

    // [{min_x, min_y, min_z}, {max_x, max_y, max_z}]
    std::pair<glm::vec3, glm::vec3> ComputeBounds() const {
        static const float min_float = std::numeric_limits<float>::lowest();
        static const float max_float = std::numeric_limits<float>::max();

        glm::vec3 min(max_float), max(min_float);
        for (const auto &v : Vertices) {
            min.x = std::min(min.x, v[0]);
            min.y = std::min(min.y, v[1]);
            min.z = std::min(min.z, v[2]);
            max.x = std::max(max.x, v[0]);
            max.y = std::max(max.y, v[1]);
            max.z = std::max(max.z, v[2]);
        }

        return {min, max};
    }

    mutable bool Dirty{true};

    std::vector<glm::vec3> Vertices;
    std::vector<uint> Indices;
    std::vector<glm::vec3> Normals;
};
