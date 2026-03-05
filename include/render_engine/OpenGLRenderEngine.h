#pragma once

#include "render_engine/IRenderEngine.h"
#include "render_engine/renderer.h"
#include "render_engine/gui_manager.h"
#include <memory>
#include <string>

struct GLFWwindow;
class Renderer;
class GUIManager;
class Camera;
struct Scene;
class Model;
class Animator;

/**
 * @class OpenGLRenderEngine
 * @brief OpenGL/GLFW 渲染引擎实现：封装窗口创建、GLAD 初始化与渲染循环。
 */
class OpenGLRenderEngine : public IRenderEngine {
public:
    OpenGLRenderEngine();
    ~OpenGLRenderEngine() override;

    bool Initialize() override;
    bool ShouldClose() const override;
    void BeginFrame() override;
    void EndFrame() override;
    void RenderFrame(Scene& scene, Camera& camera, const RenderParams& params) override;
    void RenderGui(const GUIContext& ctx) override;
    float GetDeltaTime() const override;
    int GetWidth() const override;
    int GetHeight() const override;
    bool IsKeyPressed(int key) const override;
    void SetCameraForInput(Camera* camera) override;
    bool IsSpotLightOn() const override;

private:
    void processInput();

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

    GLFWwindow* m_Window = nullptr;
    std::unique_ptr<Renderer> m_Renderer;
    std::unique_ptr<GUIManager> m_GUIManager;

    Camera* m_InputCamera = nullptr;

    int m_Width = 1920;
    int m_Height = 1080;
    float m_DeltaTime = 0.0f;
    float m_LastFrame = 0.0f;

    bool m_HideMouse = true;
    float m_LastX = 0.0f;
    float m_LastY = 0.0f;
    bool m_FirstMouse = true;
    float m_Sensitivity = 0.05f;

    bool m_SpotLightOn = false;

    int m_ShadowWidth = 2048;
    int m_ShadowHeight = 2048;
};
