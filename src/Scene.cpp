#include "Scene.h"

#include <format>
#include <string>

#include "GLCanvas.h"
#include "Mesh/Primitive/Sphere.h"
#include "Shader/ShaderProgram.h"

namespace UniformName {
inline static const std::string
    NumLights = "num_lights",
    AmbientColor = "ambient_color",
    DiffuseColor = "diffuse_color",
    SpecularColor = "specular_color",
    ShininessFactor = "shininess_factor",
    Projection = "projection",
    CameraView = "camera_view",
    FlatShading = "flat_shading";
} // namespace UniformName

Scene::Scene() {
    Canvas = std::make_unique<GLCanvas>();

    /**
      Initialize light positions using a three-point lighting system:
        1) Key light: The main light, positioned at a 45-degree angle from the subject.
        2) Fill light: Positioned opposite the key light to fill the shadows. It's less intense than the key light.
        3) Back light: Positioned behind and above the subject to create a rim of light around the subject, separating it from the background.
      We consider the "subject" to be the origin.
    */
    for (int _ = 0; _ < 3; _++) Lights.push_back({});
    static const float dist_factor = 8.0f;

    // Key light.
    float key_light__angle = 1.f / 4.f; // Multiplied by pi.
    Lights[0].Position = {dist_factor * __cospif(key_light__angle), 0, dist_factor * __sinpif(key_light__angle), 1};
    // Fill light, twice as far away to make it less intense.
    Lights[1].Position = {-dist_factor * __cospif(key_light__angle) * 2, 0, -dist_factor * __sinpif(key_light__angle) * 2, 1};
    // Back light.
    Lights[2].Position = {0, dist_factor * 1.5, -dist_factor, 1};

    /**
      Initialize a right-handed coordinate system, with:
        * Positive x pointing right
        * Positive y pointing up, and
        * Positive z pointing forward (toward the camera).
      This would put the camera `eye` at position (0, 0, camDistance) in world space, pointing at the origin.
      We offset the camera angle slightly from this point along spherical coordinates to make the initial view more interesting.
    */
    static const float x_angle = M_PI * -0.1; // Elevation angle (0° is in the X-Z plane, positive angles rotate upwards)
    static const float y_angle = M_PI * 0.6; // Azimuth angle (0° is along +X axis, positive angles rotate counterclockwise)
    static const glm::vec3 eye(cosf(y_angle) * cosf(x_angle), sinf(x_angle), sinf(y_angle) * cosf(x_angle));
    CameraView = glm::lookAt(eye * CameraDistance, Origin, Up);

    namespace un = UniformName;
    static const fs::path ShaderDir = fs::path("res") / "shaders";
    static const Shader
        TransformVertexShader{GL_VERTEX_SHADER, ShaderDir / "transform_vertex.glsl", {un::Projection, un::CameraView}},
        FragmentShader{GL_FRAGMENT_SHADER, ShaderDir / "fragment.glsl", {un::NumLights, un::AmbientColor, un::DiffuseColor, un::SpecularColor, un::ShininessFactor, un::FlatShading}};

    MainShaderProgram = std::make_unique<ShaderProgram>(std::vector<const Shader *>{&TransformVertexShader, &FragmentShader});

    CurrShaderProgram = MainShaderProgram.get();
    CurrShaderProgram->Use();

    glGenBuffers(1, &LightBufferId);
    glBindBuffer(GL_UNIFORM_BUFFER, LightBufferId);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(Light) * Lights.size(), Lights.data(), GL_STATIC_DRAW);

    GLuint light_block_index = glGetUniformBlockIndex(CurrShaderProgram->Id, "LightBlock");
    glBindBufferBase(GL_UNIFORM_BUFFER, light_block_index, LightBufferId);
}

Scene::~Scene() {
    glDeleteBuffers(1, &LightBufferId);
}

void Scene::AddMesh(Mesh *mesh) {
    if (!mesh) return;
    if (std::find(Meshes.begin(), Meshes.end(), mesh) != Meshes.end()) return;

    Meshes.push_back(mesh);
}

void Scene::RemoveMesh(const Mesh *mesh) {
    if (!mesh) return;

    Meshes.erase(std::remove(Meshes.begin(), Meshes.end(), mesh), Meshes.end());
}

void Scene::SetCameraDistance(float distance) {
    // Extract the eye position from inverse camera view matrix and update the camera view based on the new distance.
    const glm::vec3 eye = glm::inverse(CameraView)[3];
    CameraView = glm::lookAt(eye * (distance / CameraDistance), Origin, Up);
    CameraDistance = distance;
}

using namespace ImGui;

void Scene::Render() {
    const auto &io = ImGui::GetIO();
    const bool window_hovered = IsWindowHovered();
    if (window_hovered && io.MouseWheel != 0) {
        SetCameraDistance(CameraDistance * (1.f - io.MouseWheel / 16.f));
    }
    const auto content_region = GetContentRegionAvail();
    CameraProjection = glm::perspective(glm::radians(fov), content_region.x / content_region.y, 0.1f, 1000.f);

    if (content_region.x <= 0 && content_region.y <= 0) return;

    const auto bg = GetStyleColorVec4(ImGuiCol_WindowBg);
    Canvas->PrepareRender(content_region.x, content_region.y, bg.x, bg.y, bg.z, bg.w);

    glBindBuffer(GL_UNIFORM_BUFFER, LightBufferId);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(Light) * Lights.size(), Lights.data(), GL_STATIC_DRAW);

    CurrShaderProgram->Use();

    namespace un = UniformName;
    glUniformMatrix4fv(CurrShaderProgram->GetUniform(un::Projection), 1, GL_FALSE, &CameraProjection[0][0]);
    glUniformMatrix4fv(CurrShaderProgram->GetUniform(un::CameraView), 1, GL_FALSE, &CameraView[0][0]);
    glUniform1i(CurrShaderProgram->GetUniform(un::NumLights), Lights.size());
    glUniform4fv(CurrShaderProgram->GetUniform(un::AmbientColor), 1, &AmbientColor[0]);
    glUniform4fv(CurrShaderProgram->GetUniform(un::DiffuseColor), 1, &DiffusionColor[0]);
    glUniform4fv(CurrShaderProgram->GetUniform(un::SpecularColor), 1, &SpecularColor[0]);
    glUniform1f(CurrShaderProgram->GetUniform(un::ShininessFactor), Shininess);
    glUniform1i(CurrShaderProgram->GetUniform(un::FlatShading), UseFlatShading ? 1 : 0);

    // auto start_time = std::chrono::high_resolution_clock::now();
    for (const auto *mesh : Meshes) mesh->Render();
    // std::cout << "Draw time: " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start_time).count() << "us" << std::endl;

    // Render the scene to an OpenGl texture and display it (without changing the cursor position).
    const auto &cursor = GetCursorPos();
    unsigned int texture_id = Canvas->Render();
    Image((void *)(intptr_t)texture_id, content_region, {0, 1}, {1, 0});
    SetCursorPos(cursor);

    // Render ImGuizmo.
    const auto &window_pos = GetWindowPos();
    if (ShowCameraGizmo) {
        ImGuizmo::BeginFrame();
        ImGuizmo::SetDrawlist();
        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetRect(window_pos.x, window_pos.y + GetTextLineHeightWithSpacing(), content_region.x, content_region.y);

        static const float ViewManipulateSize = 128;
        const auto view_manipulate_pos = window_pos + ImVec2{GetWindowContentRegionMax().x - ViewManipulateSize, GetWindowContentRegionMin().y};
        ImGuizmo::ViewManipulate(&CameraView[0][0], CameraDistance, view_manipulate_pos, {ViewManipulateSize, ViewManipulateSize}, 0);
    }
}

void Scene::RenderConfig() {
    if (BeginTabBar("SceneConfig")) {
        if (BeginTabItem("Geometries")) {
            Checkbox("Flat shading", &UseFlatShading);
            EndTabItem();
        }
        if (BeginTabItem("Camera")) {
            Checkbox("Show gizmo", &ShowCameraGizmo);
            SameLine();
            SliderFloat("FOV", &fov, 20.f, 110.f);

            float camera_distance = CameraDistance;
            if (SliderFloat("Distance", &camera_distance, .1f, 200.f, "%.3f", ImGuiSliderFlags_Logarithmic)) {
                SetCameraDistance(camera_distance);
            }
            EndTabItem();
        }
        if (BeginTabItem("Lighting")) {
            SeparatorText("Colors");
            Checkbox("Custom colors", &CustomColors);
            if (CustomColors) {
                ColorEdit3("Ambient", &AmbientColor[0]);
                ColorEdit3("Diffusion", &DiffusionColor[0]);
                ColorEdit3("Specular", &SpecularColor[0]);
            } else {
                for (uint i = 1; i < 3; i++) {
                    AmbientColor[i] = AmbientColor[0];
                    DiffusionColor[i] = DiffusionColor[0];
                    SpecularColor[i] = SpecularColor[0];
                }
                SliderFloat("Ambient", &AmbientColor[0], 0.0f, 1.0f);
                SliderFloat("Diffusion", &DiffusionColor[0], 0.0f, 1.0f);
                SliderFloat("Specular", &SpecularColor[0], 0.0f, 1.0f);
            }
            SliderFloat("Shininess", &Shininess, 0.0f, 150.0f);

            SeparatorText("Lights");
            for (size_t i = 0; i < Lights.size(); i++) {
                Separator();
                PushID(i);
                Text("Light %d", int(i + 1));
                bool show_lights = LightPoints.contains(i);
                if (Checkbox("Show", &show_lights)) {
                    if (show_lights) {
                        LightPoints[i] = std::make_unique<Mesh>(Sphere{0.1});
                        LightPoints[i]->Generate();
                        LightPoints[i]->SetColor(Lights[i].Color);
                        LightPoints[i]->SetPosition(Lights[i].Position);
                        AddMesh(LightPoints[i].get());
                    } else {
                        RemoveMesh(LightPoints[i].get());
                        LightPoints.erase(i);
                    }
                }
                if (SliderFloat3("Position", &Lights[i].Position[0], -8, 8)) {
                    if (LightPoints.contains(i)) LightPoints[i]->SetPosition(Lights[i].Position);
                }
                if (ColorEdit3("Color", &Lights[i].Color[0]) && LightPoints.contains(i)) {
                    LightPoints[i]->SetColor(Lights[i].Color);
                }
                PopID();
            }
            EndTabItem();
        }
        EndTabBar();
    }
}
