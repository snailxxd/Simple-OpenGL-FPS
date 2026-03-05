#include "render_engine/gui_manager.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include "camera/camera.h"
#include "light/light_pbr.h"
#include "model/model_gltf.h"
#include "animation/animator.h"
#include "animation/animation.h"

GUIManager::GUIManager(GLFWwindow* window) {
    m_Window = window;
    init_gui();
}

GUIManager::~GUIManager() {
    shutdown_gui();
}

void GUIManager::init_gui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
}

void GUIManager::render_gui(const GUIContext& ctx) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Control Panel");      // 窗口标题

    float fps = ImGui::GetIO().Framerate;
    ImGui::Text("FPS: %.0f", fps);      // 显示 FPS

    if (ctx.camera) {
        ImGui::BeginChild("Camera Controls", ImVec2(0, 80), true);
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Camera Settings");
        ImGui::SliderFloat("FOV", &ctx.camera->Fov, 30.0f, 120.0f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("Camera Speed", &ctx.camera->MovementSpeed, 0.1f, 10.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::EndChild();
    }

    if (ctx.renderMode && ctx.gammaCorrection && ctx.renderSkybox) {
        ImGui::BeginChild("Render Settings", ImVec2(0, 140), true);
        ImGui::Text("Render Mode:");
        ImGui::RadioButton("Fill", ctx.renderMode, 0);
        ImGui::SameLine();
        ImGui::RadioButton("Wireframe", ctx.renderMode, 1);

        ImGui::Text("Gamma Correction");
        ImGui::RadioButton("On", ctx.gammaCorrection, 0);
        ImGui::SameLine();
        ImGui::RadioButton("Off", ctx.gammaCorrection, 1);

        ImGui::Text("Render Skybox:");
        ImGui::RadioButton("No", ctx.renderSkybox, 0);
        ImGui::SameLine();
        ImGui::RadioButton("Yes", ctx.renderSkybox, 1);
        ImGui::EndChild();
    }

    if (ctx.dirLight) {
        ImGui::BeginChild("Light Controls", ImVec2(0, 100), true);
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Light Controls");
        ImGui::ColorEdit3("Light Color", (float*)&ctx.dirLight->color);
        ImGui::SliderFloat("Light Intensity", &ctx.dirLight->intensity, 0.0f, 10.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::EndChild();
    }

    if (ctx.orthoSize && ctx.nearPlane && ctx.farPlane) {
        ImGui::BeginChild("Shadow Controls", ImVec2(0, 100), true);
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Shadow Controls");
        ImGui::SliderFloat("Ortho Size", ctx.orthoSize, 0.0f, 15.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("Near Plane", ctx.nearPlane, 0.0f, 20.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("Far Plane", ctx.farPlane, 0.0f, 20.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::EndChild();
    }

    if (ctx.model && ctx.animator && ctx.currentAnimationName) {
        ImGui::BeginChild("Animation Controls", ImVec2(0, 80), true);
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Animation Controls");
        if (!ctx.model->animations.empty()) {
            if (ImGui::BeginCombo("Animation", ctx.currentAnimationName->c_str())) {
                for (auto it = ctx.model->animations.begin(); it != ctx.model->animations.end(); ++it) {
                    bool isSelected = (*ctx.currentAnimationName == it->first);
                    if (ImGui::Selectable(it->first.c_str(), isSelected)) {
                        *ctx.currentAnimationName = it->first;
                        ctx.animator->PlayAnimation(&it->second);
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::SliderFloat("Animation Speed", &ctx.animator->GetSpeed(), 0.1f, 3.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        }
        ImGui::EndChild();
    }

    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUIManager::shutdown_gui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
