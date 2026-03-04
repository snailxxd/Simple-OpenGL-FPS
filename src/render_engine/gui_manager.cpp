#include "render_engine/gui_manager.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include "camera/camera.h"
#include "render_engine/engine_globals.h"

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
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
}

void GUIManager::render_gui() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Control Panel");      // 窗口标题

    float fps = ImGui::GetIO().Framerate;
    ImGui::Text("FPS: %.0f", fps);      // 显示 FPS

    // 摄像机设置
    ImGui::BeginChild("Camera Controls", ImVec2(0, 80), true);
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Camera Settings");
    Camera& camera = *(Camera*)glfwGetWindowUserPointer(m_Window);
    ImGui::SliderFloat("FOV", &camera.Fov, 30.0f, 120.0f, "%.0f", ImGuiSliderFlags_AlwaysClamp);  // fov 控制滑块
    ImGui::SliderFloat("Camera Speed", &camera.MovementSpeed, 0.1f, 10.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp); // 摄像机速度滑块
    ImGui::EndChild();

    // 渲染设置
    ImGui::BeginChild("Render Settings", ImVec2(0, 140), true);
    ImGui::Text("Render Mode:");
    ImGui::RadioButton("Fill", &renderMode, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Wireframe", &renderMode, 1);

    ImGui::Text("Gamma Correction");
    ImGui::RadioButton("On", &gammaCorrection, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Off", &gammaCorrection, 1);

    ImGui::Text("Render Skybox:");
    ImGui::RadioButton("No", &renderSkybox, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Yes", &renderSkybox, 1);

    ImGui::EndChild();

    // 光照设置
    ImGui::BeginChild("Light Controls", ImVec2(0, 100), true);
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Light Controls");

    ImGui::ColorEdit3("Light Color", (float*)&dirlight.color);
    ImGui::SliderFloat("Light Intensity", &dirlight.intensity, 0.0f, 10.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::EndChild();

    // 阴影设置
    ImGui::BeginChild("Shadow Controls", ImVec2(0, 100), true);
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Shadow Controls");

    ImGui::SliderFloat("Ortho Size", &orthoSize, 0.0f, 15.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::SliderFloat("Near Plane", &nearPlane, 0.0f, 20.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::SliderFloat("Far Plane", &farPlane, 0.0f, 20.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::EndChild();

    // 动画控制
    ImGui::BeginChild("Animation Controls", ImVec2(0, 80), true);
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Animation Controls");
    if (ImGui::BeginCombo("Animation", currentAnimation->first.c_str())) {
        for (auto it = ourModel->animations.begin(); it != ourModel->animations.end(); ++it) {
            bool isSelected = (it == currentAnimation);
            if (ImGui::Selectable(it->first.c_str(), isSelected)) {
                currentAnimation = it;
                animator->PlayAnimation(&currentAnimation->second);
            }
        }
        ImGui::EndCombo();
    }
    ImGui::SliderFloat("Animation Speed", &animator->GetSpeed(), 0.1f, 3.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::EndChild();

    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUIManager::shutdown_gui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}