#pragma once

#include <string>
#include <vector>

struct GLFWwindow;

#include <glm/glm.hpp>

#include "shader/shader.h"
#include "camera/camera.h"
#include "render_engine/scene.h"
#include "animation/animator.h"

/// 每帧渲染参数，由引擎传入
struct RenderParams {
    int width = 1920;
    int height = 1080;
    int shadowWidth = 2048;
    int shadowHeight = 2048;
    int renderMode = 0;       // 0: Fill, 1: Wireframe
    int gammaCorrection = 0;  // 0: On, 1: Off
    bool spotLightOn = false;
    int renderSkybox = 0;     // 0: No, 1: Yes
    float orthoSize = 2.5f;
    float nearPlane = 1.0f;
    float farPlane = 20.0f;
    glm::vec4 bgColor{0.02f, 0.02f, 0.02f, 1.0f};
    Animator* animator = nullptr;
};

// 天空盒数据
struct SkyboxData {
    unsigned int vao = 0;
    unsigned int cubemapID = 0;
};

class Renderer {
public:
    Renderer(int width, int height, int shadowWidth, int shadowHeight);
    ~Renderer();

    void BeginFrame(const RenderParams& params);
    void RenderShadows(Scene& scene, const RenderParams& params);
    void RenderScene(Scene& scene, Camera& camera, const RenderParams& params);
    void RenderSkybox(Scene& scene, Camera& camera, const RenderParams& params);
    void EndFrame(const RenderParams& params);
    void Resize(int width, int height);

private:
    // 着色器
    Shader m_PbrShader;     // 主场景着色器
    Shader m_ScreenShader;  // 帧缓冲着色器
    Shader m_SkyboxShader;  // 天空盒着色器
    Shader m_ShadowShader;  // 阴影贴图着色器

    // 帧缓冲与纹理
    unsigned int m_FBO = 0;
    unsigned int m_TexColorBuffer = 0;
    unsigned int m_TexDepthBuffer = 0;

    // 阴影 FBO 与深度贴图
    unsigned int m_ShadowFBO = 0;
    unsigned int m_DepthMap = 0;

    // 屏幕空间 Quad
    unsigned int m_ScreenVAO = 0;

    // 天空盒数据
    SkyboxData m_Skybox;

    // 光空间矩阵
    glm::mat4 m_LightSpaceMatrix{1.0f};

    int m_Width = 1920;
    int m_Height = 1080;
    int m_ShadowWidth = 2048;
    int m_ShadowHeight = 2048;

private:
    void renderSceneGeometry(Shader& shader, Scene& scene);
    glm::mat4 computeLightSpaceMatrix(const glm::vec3& lightDir, float orthoSize, float nearPlane, float farPlane) const;
    SkyboxData loadHDRtoSkybox(const std::string& hdrPath, int viewportWidth, int viewportHeight);
    unsigned int init_screenVAO();
    unsigned int init_framebuffer(unsigned int& texColorBuffer, unsigned int& texDepthBuffer, int width, int height);
    unsigned int init_shadow_fbo(unsigned int& depthMap, int shadowWidth, int shadowHeight);

    // 将场景颜色纹理绘制到屏幕
    void render_screen();
};

