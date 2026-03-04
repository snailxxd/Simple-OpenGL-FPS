#pragma once

#include <string>
#include <vector>

struct GLFWwindow;

#include <glm/glm.hpp>

#include "shader/shader.h"
#include "camera/camera.h"
#include "render_engine/scene.h"

// 天空盒数据
struct SkyboxData {
    unsigned int vao = 0;
    unsigned int cubemapID = 0;
};

class Renderer {
public:
    Renderer();
    ~Renderer();

    // 设置全局渲染状态
    void BeginFrame();

    // 渲染阴影贴图
    void RenderShadows(Scene& scene);

    // 渲染主场景到帧缓冲
    void RenderScene(Scene& scene, Camera& camera);

    // 渲染天空盒
    void RenderSkybox(Scene& scene, Camera& camera);

    // 将帧缓冲绘制到屏幕
    void EndFrame();

    // 窗口大小变化时重建主场景 FBO 纹理
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

private:
    // 场景几何渲染
    void renderSceneGeometry(Shader& shader, Scene& scene);

    // 计算光空间矩阵
    glm::mat4 computeLightSpaceMatrix(const glm::vec3& lightDir) const;

    // 资源初始化
    SkyboxData loadHDRtoSkybox(const std::string& hdrPath);
    unsigned int init_screenVAO();
    unsigned int init_framebuffer(unsigned int& texColorBuffer, unsigned int& texDepthBuffer);
    unsigned int init_shadow_fbo(unsigned int& depthMap);

    // 将场景颜色纹理绘制到屏幕
    void render_screen();
};

