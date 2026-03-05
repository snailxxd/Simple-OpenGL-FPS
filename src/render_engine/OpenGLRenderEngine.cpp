#include "render_engine/OpenGLRenderEngine.h"
#include "render_engine/renderer.h"
#include "render_engine/gui_manager.h"
#include "render_engine/scene.h"
#include "camera/camera.h"

#include <iostream>
#include <cmath>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace {
    OpenGLRenderEngine* s_Engine = nullptr;
}

OpenGLRenderEngine::OpenGLRenderEngine() = default;

OpenGLRenderEngine::~OpenGLRenderEngine() {
    s_Engine = nullptr;
    if (m_Window) {
        glfwDestroyWindow(m_Window);
        m_Window = nullptr;
    }
    glfwTerminate();
}

bool OpenGLRenderEngine::Initialize() {
    s_Engine = this;

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

    m_LastX = m_Width / 2.0f;
    m_LastY = m_Height / 2.0f;

    glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glEnable(GL_CULL_FACE);

    return true;
}

bool OpenGLRenderEngine::ShouldClose() const {
    return m_Window ? glfwWindowShouldClose(m_Window) : true;
}

void OpenGLRenderEngine::BeginFrame() {
    float currentFrame = static_cast<float>(glfwGetTime());
    m_DeltaTime = currentFrame - m_LastFrame;
    m_LastFrame = currentFrame;

    processInput();
}

void OpenGLRenderEngine::EndFrame() {
    if (m_Window) {
        glfwSwapBuffers(m_Window);
        glfwPollEvents();
    }
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

void OpenGLRenderEngine::SetCameraForInput(Camera* camera) {
    m_InputCamera = camera;
}

bool OpenGLRenderEngine::IsSpotLightOn() const {
    return m_SpotLightOn;
}

void OpenGLRenderEngine::processInput() {
    if (!m_InputCamera || !m_Window) return;
    Camera& camera = *m_InputCamera;
    if (glfwGetKey(m_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(m_Window, true);
    if (glfwGetKey(m_Window, GLFW_KEY_W) == GLFW_PRESS)
        camera.Move(HORIZON_FORWARD, m_DeltaTime);
    if (glfwGetKey(m_Window, GLFW_KEY_S) == GLFW_PRESS)
        camera.Move(HORIZON_BACKWARD, m_DeltaTime);
    if (glfwGetKey(m_Window, GLFW_KEY_A) == GLFW_PRESS)
        camera.Move(LEFT, m_DeltaTime);
    if (glfwGetKey(m_Window, GLFW_KEY_D) == GLFW_PRESS)
        camera.Move(RIGHT, m_DeltaTime);
    if (glfwGetKey(m_Window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.Move(WORLD_UP, m_DeltaTime);
    if (glfwGetKey(m_Window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        camera.Move(WORLD_DOWN, m_DeltaTime);
}

void OpenGLRenderEngine::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    if (s_Engine) {
        s_Engine->m_Width = width;
        s_Engine->m_Height = height;
        if (s_Engine->m_Renderer) {
            s_Engine->m_Renderer->Resize(width, height);
        }
    }
}

void OpenGLRenderEngine::mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    (void)window;
    if (!s_Engine || !s_Engine->m_InputCamera) return;
    if (!s_Engine->m_HideMouse) {
        s_Engine->m_FirstMouse = true;
        return;
    }
    if (s_Engine->m_FirstMouse) {
        s_Engine->m_LastX = static_cast<float>(xpos);
        s_Engine->m_LastY = static_cast<float>(ypos);
        s_Engine->m_FirstMouse = false;
    }
    float xoffset = static_cast<float>(xpos) - s_Engine->m_LastX;
    float yoffset = s_Engine->m_LastY - static_cast<float>(ypos);
    s_Engine->m_LastX = static_cast<float>(xpos);
    s_Engine->m_LastY = static_cast<float>(ypos);
    s_Engine->m_InputCamera->PitchAndYaw(xoffset * s_Engine->m_Sensitivity, yoffset * s_Engine->m_Sensitivity);
}

void OpenGLRenderEngine::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void)scancode;
    (void)mods;
    if (!s_Engine) return;
    if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS) {
        s_Engine->m_HideMouse = !s_Engine->m_HideMouse;
        if (s_Engine->m_HideMouse) {
            s_Engine->m_FirstMouse = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
    if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        s_Engine->m_SpotLightOn = !s_Engine->m_SpotLightOn;
    }
}
