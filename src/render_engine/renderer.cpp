#include "render_engine/renderer.h"

#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <glm/gtc/matrix_transform.hpp>

#include "model/model_gltf.h"

#define RESOURCES_PATH(path) (std::string(RESOURCES_DIR) + path)

Renderer::Renderer(int width, int height, int shadowWidth, int shadowHeight)
    : m_Width(width), m_Height(height), m_ShadowWidth(shadowWidth), m_ShadowHeight(shadowHeight),
      m_PbrShader(RESOURCES_PATH("/shaders/core/pbr.vs"), RESOURCES_PATH("/shaders/core/pbr.fs")),
      m_ScreenShader(RESOURCES_PATH("/shaders/frame/frame.vs"), RESOURCES_PATH("/shaders/frame/frame.fs")),
      m_SkyboxShader(RESOURCES_PATH("/shaders/skybox/skybox.vs"), RESOURCES_PATH("/shaders/skybox/skybox.fs")),
      m_ShadowShader(RESOURCES_PATH("/shaders/core/shadow.vs"), RESOURCES_PATH("/shaders/core/shadow.fs")) {

    m_Skybox = loadHDRtoSkybox(RESOURCES_PATH("/hdr/citrus_orchard_puresky_1k.hdr"), width, height);
    m_ScreenVAO = init_screenVAO();
    m_FBO = init_framebuffer(m_TexColorBuffer, m_TexDepthBuffer, width, height);
    m_ShadowFBO = init_shadow_fbo(m_DepthMap, shadowWidth, shadowHeight);
}

Renderer::~Renderer() {
    if (m_FBO) {
        glDeleteFramebuffers(1, &m_FBO);
    }
    if (m_TexColorBuffer) {
        glDeleteTextures(1, &m_TexColorBuffer);
    }
    if (m_TexDepthBuffer) {
        glDeleteTextures(1, &m_TexDepthBuffer);
    }
    if (m_ShadowFBO) {
        glDeleteFramebuffers(1, &m_ShadowFBO);
    }
    if (m_DepthMap) {
        glDeleteTextures(1, &m_DepthMap);
    }
    if (m_ScreenVAO) {
        glDeleteVertexArrays(1, &m_ScreenVAO);
    }
    if (m_Skybox.vao) {
        glDeleteVertexArrays(1, &m_Skybox.vao);
    }
    if (m_Skybox.cubemapID) {
        glDeleteTextures(1, &m_Skybox.cubemapID);
    }
}

void Renderer::BeginFrame(const RenderParams& params) {
    if (params.renderMode)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

void Renderer::RenderShadows(Scene& scene, const RenderParams& params) {
    m_LightSpaceMatrix = computeLightSpaceMatrix(scene.dirLight.direction, params.orthoSize, params.nearPlane, params.farPlane);

    glViewport(0, 0, params.shadowWidth, params.shadowHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, m_ShadowFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    m_ShadowShader.use();
    if (params.animator) {
        auto transforms = params.animator->GetFinalBoneMatrices();
        for (int i = 0; i < static_cast<int>(transforms.size()); ++i) {
            m_ShadowShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
        }
    }
    m_ShadowShader.setMat4("lightSpaceMatrix", m_LightSpaceMatrix);

    renderSceneGeometry(m_ShadowShader, scene);

    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::RenderScene(Scene& scene, Camera& camera, const RenderParams& params) {
    glViewport(0, 0, params.width, params.height);

    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
    glClearColor(params.bgColor.r, params.bgColor.g, params.bgColor.b, params.bgColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_PbrShader.use();
    m_PbrShader.setMat4("lightSpaceMatrix", m_LightSpaceMatrix);

    scene.spotLight.position = camera.Position;
    scene.spotLight.direction = camera.Front;
    scene.dirLight.setUniform(m_PbrShader);
    scene.spotLight.setUniform(m_PbrShader);
    if (!params.spotLightOn) {
        scene.spotLight.shut(m_PbrShader);
    }

    m_PbrShader.setVec3("viewPos", camera.Position);

    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = camera.GetProjectionMatrix(static_cast<float>(params.width) / static_cast<float>(params.height));
    m_PbrShader.setMat4("view", view);
    m_PbrShader.setMat4("projection", projection);

    if (params.animator) {
        auto transforms = params.animator->GetFinalBoneMatrices();
        for (int i = 0; i < static_cast<int>(transforms.size()); ++i) {
            m_PbrShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
        }
    }

    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, m_DepthMap);
    m_PbrShader.setInt("dirLight.shadowMap", 6);

    renderSceneGeometry(m_PbrShader, scene);
}

void Renderer::RenderSkybox(Scene& scene, Camera& camera, const RenderParams& params) {
    (void)scene;

    if (!params.renderSkybox) {
        return;
    }

    glm::mat4 projection = camera.GetProjectionMatrix(static_cast<float>(params.width) / static_cast<float>(params.height));
    glm::mat4 view = camera.GetViewMatrix();

    glDepthFunc(GL_LEQUAL);
    m_SkyboxShader.use();

    glm::mat4 skyView = glm::mat4(glm::mat3(view));
    m_SkyboxShader.setMat4("view", skyView);
    m_SkyboxShader.setMat4("projection", projection);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_Skybox.cubemapID);
    m_SkyboxShader.setInt("skybox", 0);

    glBindVertexArray(m_Skybox.vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthFunc(GL_LESS);
}

void Renderer::EndFrame(const RenderParams& params) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    if (!params.gammaCorrection) {
        glEnable(GL_FRAMEBUFFER_SRGB);
    }

    render_screen();

    glDisable(GL_FRAMEBUFFER_SRGB);
}

void Renderer::Resize(int width, int height) {
    m_Width = width;
    m_Height = height;

    if (m_FBO) {
        glDeleteFramebuffers(1, &m_FBO);
        m_FBO = 0;
    }
    if (m_TexColorBuffer) {
        glDeleteTextures(1, &m_TexColorBuffer);
        m_TexColorBuffer = 0;
    }
    if (m_TexDepthBuffer) {
        glDeleteTextures(1, &m_TexDepthBuffer);
        m_TexDepthBuffer = 0;
    }

    m_FBO = init_framebuffer(m_TexColorBuffer, m_TexDepthBuffer, width, height);
}

void Renderer::renderSceneGeometry(Shader& shader, Scene& scene) {
    if (scene.GetEntityCount() == 0) {
        return;
    }
    shader.use();
    for (auto entity : scene.GetEntities()) {
        if (!entity->model) {
            continue;
        }
        glm::mat4 transform = entity->GetModelMatrix();
        entity->model->Draw(shader, transform);
    }
}

glm::mat4 Renderer::computeLightSpaceMatrix(const glm::vec3& lightDir, float orthoSize, float nearPlane, float farPlane) const {
    glm::mat4 lightProjection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, nearPlane, farPlane);
    glm::mat4 lightView = glm::lookAt(lightDir * -10.0f, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    return lightProjection * lightView;
}

SkyboxData Renderer::loadHDRtoSkybox(const std::string& hdrPath, int viewportWidth, int viewportHeight) {
    SkyboxData result{};

    float vertices[] = {
        -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f
    };
    unsigned int vbo;
    glGenVertexArrays(1, &result.vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(result.vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // 加载 HDR 2D 纹理
    stbi_set_flip_vertically_on_load(true);
    int width, height, nrComponents;
    float* data = stbi_loadf(hdrPath.c_str(), &width, &height, &nrComponents, 0);
    unsigned int hdrTexture = 0;
    if (data) {
        glGenTextures(1, &hdrTexture);
        glBindTexture(GL_TEXTURE_2D, hdrTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
    } else {
        std::cout << "Failed to load HDR: " << hdrPath << std::endl;
        return result;
    }
    stbi_set_flip_vertically_on_load(false);

    // 创建目标 Cubemap
    glGenTextures(1, &result.cubemapID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, result.cubemapID);
    for (unsigned int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 离屏渲染转换
    unsigned int captureFBO, captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

    // 转换矩阵
    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] = {
        glm::lookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3( 1.0,  0.0,  0.0), glm::vec3(0.0, -1.0,  0.0)),
        glm::lookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3(-1.0,  0.0,  0.0), glm::vec3(0.0, -1.0,  0.0)),
        glm::lookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3( 0.0,  1.0,  0.0), glm::vec3(0.0,  0.0,  1.0)),
        glm::lookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3( 0.0, -1.0,  0.0), glm::vec3(0.0,  0.0, -1.0)),
        glm::lookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3( 0.0,  0.0,  1.0), glm::vec3(0.0, -1.0,  0.0)),
        glm::lookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3( 0.0,  0.0, -1.0), glm::vec3(0.0, -1.0,  0.0))
    };

    // 使用转换着色器
    Shader convertShader(RESOURCES_PATH("/shaders/skybox/equirect_to_cubemap.vs"),
                         RESOURCES_PATH("/shaders/skybox/equirect_to_cubemap.fs"));
    convertShader.use();
    convertShader.setInt("equirectangularMap", 0);
    convertShader.setMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);

    glViewport(0, 0, 512, 512);
    for (unsigned int i = 0; i < 6; ++i) {
        convertShader.setMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, result.cubemapID, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindVertexArray(result.vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // 清理临时资源
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteTextures(1, &hdrTexture);
    glDeleteFramebuffers(1, &captureFBO);
    glDeleteRenderbuffers(1, &captureRBO);
    glViewport(0, 0, viewportWidth, viewportHeight);

    return result;
}

unsigned int Renderer::init_framebuffer(unsigned int& texColorBuffer, unsigned int& texDepthBuffer, int width, int height) {
    unsigned int FBO;
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    glGenTextures(1, &texColorBuffer);
    glBindTexture(GL_TEXTURE_2D, texColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);

    glGenTextures(1, &texDepthBuffer);
    glBindTexture(GL_TEXTURE_2D, texDepthBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texDepthBuffer, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return FBO;
}

void Renderer::render_screen() {
    m_ScreenShader.use();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBindVertexArray(m_ScreenVAO);
    glBindTexture(GL_TEXTURE_2D, m_TexColorBuffer);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

unsigned int Renderer::init_screenVAO() {
    float quadVertices[] = {
        // Positions   // TexCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);

    return quadVAO;
}

unsigned int Renderer::init_shadow_fbo(unsigned int& depthMap, int shadowWidth, int shadowHeight) {
    unsigned int FBO;
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowWidth, shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return FBO;
}

