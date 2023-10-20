#pragma once

#include <functional>
#include <unordered_map>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"

#include "ImGuizmo.h"

#include "Mesh/Mesh.h"

struct GLCanvas;
struct ShaderProgram;
struct Rect;
struct Physics;

struct Light {
    glm::vec4 Position{0.0f};
    glm::vec4 Color{1.0f};
};

struct Scene {
    Scene();
    ~Scene();

    void AddMesh(Mesh *);
    void RemoveMesh(const Mesh *);

    void Render();
    void RenderConfig();

    void SetCameraDistance(float);

    std::vector<Mesh *> Meshes;

    GLuint LightBufferId;
    std::vector<Light> Lights;
    glm::vec4 AmbientColor = {0.4, 0.4, 0.4, 1};
    // todo Diffusion and specular colors are object properties, not scene properties.
    glm::vec4 DiffusionColor = {0.5, 0.5, 0.5, 1};
    glm::vec4 SpecularColor = {0.0, 0.0, 0.0, 1}; // No specular by default.
    float Shininess = 10;
    bool CustomColors = false, FlatShading = false;

    bool ShowCameraGizmo = true;

    glm::mat4 CameraView, CameraProjection;
    float CameraDistance = 4, fov = 50;

    inline static float Bounds[6] = {-0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f};

    std::unique_ptr<ShaderProgram> MainShaderProgram;

    ShaderProgram *CurrShaderProgram = nullptr;

    std::unordered_map<uint, std::unique_ptr<Mesh>> LightPoints; // For visualizing light positions. Key is `Lights` index.

    std::unique_ptr<GLCanvas> Canvas;
};
