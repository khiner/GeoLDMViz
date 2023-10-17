#pragma once

#include "Geometry.h"

struct GLVertexArray {
    void Generate() { glGenVertexArrays(1, &Id); }
    void Delete() const { glDeleteVertexArrays(1, &Id); }
    void Bind() const { glBindVertexArray(Id); }
    void Unbind() const { glBindVertexArray(0); }

    uint Id = 0;
};

struct Mesh {
    Mesh(Geometry &&triangles) : Triangles(std::move(triangles)) {}
    virtual ~Mesh() {}

    const glm::mat4 &GetTransform() const { return Transforms[0]; }

    void Generate();
    void Delete() const;
    void EnableVertexAttributes() const;

    void Render() const;

    uint NumInstances() const { return Transforms.size(); }

    std::pair<glm::vec3, glm::vec3> ComputeBounds() const {
        auto [min, max] = Triangles.ComputeBounds();
        for (const auto &transform : Transforms) {
            for (int i = 0; i < 3; i++) {
                auto transformed = transform * glm::vec4(min, 1);
                min[i] = std::min(min[i], transformed[i]);
                max[i] = std::max(max[i], transformed[i]);
            }
            for (int i = 0; i < 3; i++) {
                auto transformed = transform * glm::vec4(max, 1);
                min[i] = std::min(min[i], transformed[i]);
                max[i] = std::max(max[i], transformed[i]);
            }
        }

        return {min, max};
    }

    void AddInstance() {
        Transforms.emplace_back(1);
        Colors.emplace_back(1);
        Dirty = true;
    }
    void ClearInstances() {
        Transforms.clear();
        Colors.clear();
        Dirty = true;
    }

    void SetPosition(uint instance, const glm::vec3 &position) {
        auto &transform = Transforms[instance];
        transform[3][0] = position.x;
        transform[3][1] = position.y;
        transform[3][2] = position.z;
        Dirty = true;
    }
    void SetPosition(const glm::vec3 &position) {
        for (uint instance = 0; instance < Transforms.size(); instance++) SetPosition(instance, position);
    }
    void SetScale(uint instance, float scale) {
        auto &transform = Transforms[instance];
        transform[0][0] = scale;
        transform[1][1] = scale;
        transform[2][2] = scale;
        Dirty = true;
    }
    void SetScale(float scale) {
        for (uint instance = 0; instance < Transforms.size(); instance++) SetScale(instance, scale);
    }
    void SetTransform(uint instance, const glm::mat4 &new_transform) {
        Transforms[instance] = new_transform;
        Dirty = true;
    }
    void SetTransform(const glm::mat4 &new_transform) {
        for (uint instance = 0; instance < Transforms.size(); instance++) SetTransform(instance, new_transform);
    }
    void SetTransforms(std::vector<glm::mat4> &&transforms) {
        Transforms = std::move(transforms);
        Dirty = true;
    }
    void ClearTransforms() {
        Transforms.clear();
        Dirty = true;
    }
    void SetColor(uint instance, const glm::vec4 &color) {
        Colors[instance] = color;
        Dirty = true;
    }
    void SetColor(const glm::vec4 &color) {
        Colors.clear();
        Colors.resize(Transforms.size(), color);
        Dirty = true;
    }
    void SetColors(std::vector<glm::vec4> &&colors) {
        Colors = std::move(colors);
        Colors.resize(Transforms.size());
        Dirty = true;
    }
    void ClearColors() {
        Colors.clear();
        Dirty = true;
    }

    Geometry Triangles;

private:
    std::vector<glm::vec4> Colors{{1, 1, 1, 1}};
    std::vector<glm::mat4> Transforms{glm::mat4{1}};

    GLVertexArray VertexArray;
    GLBuffer<glm::vec4, GL_ARRAY_BUFFER> ColorBuffer;
    GLBuffer<glm::mat4, GL_ARRAY_BUFFER> TransformBuffer;
    mutable bool Dirty{true};

    void BindData() const;
};
