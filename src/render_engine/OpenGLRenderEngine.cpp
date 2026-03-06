#include "render_engine/OpenGLRenderEngine.h"
#include "render_engine/renderer.h"
#include "render_engine/gui_manager.h"
#include "world/scene.h"
#include "camera/camera.h"

#include <iostream>
#include <cmath>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace {
    OpenGLRenderEngine* s_Engine = nullptr;
}

OpenGLRenderEngine::OpenGLRenderEngine() {
    s_Engine = this;
}

OpenGLRenderEngine::~OpenGLRenderEngine() {
    s_Engine = nullptr;
    if (m_Window) {
        glfwDestroyWindow(m_Window);
        m_Window = nullptr;
    }
    glfwTerminate();
}

bool OpenGLRenderEngine::Initialize() {
    // 初始化 GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口
    m_Window = glfwCreateWindow(m_Width, m_Height, "Simple-OpenGL-FPS", nullptr, nullptr);
    if (!m_Window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(m_Window);

    // 绑定回调函数
    glfwSetFramebufferSizeCallback(m_Window, framebuffer_size_callback);
    glfwSetCursorPosCallback(m_Window, mouse_callback);
    glfwSetKeyCallback(m_Window, key_callback);

    // 加载 GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    // 创建渲染器与 GUI 管理器
    m_Renderer = std::make_unique<Renderer>(m_Width, m_Height, m_ShadowWidth, m_ShadowHeight);
    m_GUIManager = std::make_unique<GUIManager>(m_Window);

    SetMouseHidden(true);
    glEnable(GL_CULL_FACE);

    return true;
}

void OpenGLRenderEngine::BeginFrame() {
    float currentFrame = static_cast<float>(glfwGetTime());
    m_DeltaTime = currentFrame - m_LastFrame;
    m_LastFrame = currentFrame;

    m_MouseDeltaX = 0.0f;
    m_MouseDeltaY = 0.0f;

    glfwPollEvents();
}

void OpenGLRenderEngine::EndFrame() {
    glfwSwapBuffers(m_Window);
}

void OpenGLRenderEngine::RenderFrame(Scene& scene, Camera& camera, const RenderParams& params) {
    if (!m_Renderer) return;
    m_Renderer->BeginFrame(params);
    m_Renderer->RenderShadows(scene, params);
    m_Renderer->RenderScene(scene, camera, params);
    m_Renderer->RenderSkybox(scene, camera, params);
    m_Renderer->EndFrame(params);
}

void OpenGLRenderEngine::RenderGui(const GUIContext& ctx) {
    if (!m_GUIManager) return;
    m_GUIManager->render_gui(ctx);
}

float OpenGLRenderEngine::GetDeltaTime() const {
    return m_DeltaTime;
}

int OpenGLRenderEngine::GetWidth() const {
    return m_Width;
}

int OpenGLRenderEngine::GetHeight() const {
    return m_Height;
}

bool OpenGLRenderEngine::IsKeyPressed(int key) const {
    if (!m_Window) return false;
    return glfwGetKey(m_Window, key) == GLFW_PRESS;
}

void OpenGLRenderEngine::GetMouseDelta(float& x, float& y) {
    x = m_MouseDeltaX;
    y = m_MouseDeltaY;
}

void OpenGLRenderEngine::SetMouseHidden(bool hidden) {
    m_HideMouse = hidden;
    if (hidden) {
        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        m_FirstMouse = true;
    } else {
        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

bool OpenGLRenderEngine::ShouldClose() const {
    return m_Window ? glfwWindowShouldClose(m_Window) : true;
}

void OpenGLRenderEngine::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    if (s_Engine) {
        s_Engine->m_Width = width;
        s_Engine->m_Height = height;
        glViewport(0, 0, width, height);
        if (s_Engine->m_Renderer) {
            s_Engine->m_Renderer->Resize(width, height);
        }
    }
}

void OpenGLRenderEngine::mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    (void)window;
    if (!s_Engine || !s_Engine->m_HideMouse) return;

    if (s_Engine->m_FirstMouse) {
        s_Engine->m_LastX = static_cast<float>(xpos);
        s_Engine->m_LastY = static_cast<float>(ypos);
        s_Engine->m_FirstMouse = false;
    }

    s_Engine->m_MouseDeltaX = static_cast<float>(xpos) - s_Engine->m_LastX;
    s_Engine->m_MouseDeltaY = s_Engine->m_LastY - static_cast<float>(ypos);

    s_Engine->m_LastX = static_cast<float>(xpos);
    s_Engine->m_LastY = static_cast<float>(ypos);
}

void OpenGLRenderEngine::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}
