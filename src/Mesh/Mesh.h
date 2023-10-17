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
    Mesh(Geometry &&triangles) : Triangles(std::move(triangles)) {
        Generate();
    }
    virtual ~Mesh() {}

    const glm::mat4 &GetTransform() const { return Transforms[0]; }

    void Generate();
    void Delete() const;
    void EnableVertexAttributes() const;

    void Render() const;

    void SetPosition(const glm::vec3 &position) {
        for (auto &transform : Transforms) {
            transform[3][0] = position.x;
            transform[3][1] = position.y;
            transform[3][2] = position.z;
        }
        Dirty = true;
    }
    void SetTransform(const glm::mat4 &new_transform) {
        for (auto &transform : Transforms) transform = new_transform;
        Dirty = true;
    }
    void SetTransforms(std::vector<glm::mat4> &&transforms) {
        Transforms = std::move(transforms);
        Dirty = true;
    }
    void ClearTransforms() {
        Transforms.clear();
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
