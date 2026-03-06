#pragma once

#include "render_engine/IRenderEngine.h"
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
    void BeginFrame() override;
    void EndFrame() override;

    void RenderFrame(Scene& scene, Camera& camera, const RenderParams& params) override;
    void RenderGui(const GUIContext& ctx) override;

    float GetDeltaTime() const override;
    int GetWidth() const override;
    int GetHeight() const override;
    int GetShadowWidth() const override;
    int GetShadowHeight() const override;
    bool ShouldClose() const override;

    bool IsKeyPressed(int key) const override;
    bool IsMousePressed(int button) const override;
    void GetMouseDelta(float& x, float& y) override;
    void SetMouseHidden(bool hidden) override;

private:
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

    GLFWwindow* m_Window = nullptr;
    std::unique_ptr<Renderer> m_Renderer;
    std::unique_ptr<GUIManager> m_GUIManager;

    int m_Width = 1920;
    int m_Height = 1080;
    float m_DeltaTime = 0.0f;
    float m_LastFrame = 0.0f;

    float m_LastX = 0.0f;
    float m_LastY = 0.0f;
    bool m_FirstMouse = true;
    float m_MouseDeltaX = 0.0f;
    float m_MouseDeltaY = 0.0f;
    bool m_HideMouse = true;

    int m_ShadowWidth = 4096;
    int m_ShadowHeight = 4096;
};
