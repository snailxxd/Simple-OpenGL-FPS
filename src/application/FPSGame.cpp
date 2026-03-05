#include "application/FPSGame.h"

#include "model/model_gltf.h"
#include "animation/animator.h"

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

#define RESOURCES_PATH(path) (std::string(RESOURCES_DIR) + path)

void FPSGame::OnInit() {
    // 模型与动画
    m_Model = std::make_unique<Model>(RESOURCES_PATH("/models/fps_ak-74m_animations.glb"), true);
    if (!m_Model->animations.empty()) {
        m_Animator = std::make_unique<Animator>(&m_Model->animations.begin()->second);
        m_CurrentAnimationName = m_Model->animations.begin()->first;
    }

    // 场景挂接
    m_Scene.models.push_back(m_Model.get());
    if (m_Animator) {
        m_Scene.animators.push_back(m_Animator.get());
    }

    // 将相机指针交给渲染引擎用于处理输入/鼠标
    GetEngine().SetCameraForInput(&m_Camera);

    // 初始化渲染参数
    m_RenderParams.width = GetEngine().GetWidth();
    m_RenderParams.height = GetEngine().GetHeight();
    m_RenderParams.shadowWidth = 2048;
    m_RenderParams.shadowHeight = 2048;
    m_RenderParams.renderMode = m_RenderMode;
    m_RenderParams.gammaCorrection = m_GammaCorrection;
    m_RenderParams.renderSkybox = m_RenderSkybox;
    m_RenderParams.orthoSize = m_OrthoSize;
    m_RenderParams.nearPlane = m_NearPlane;
    m_RenderParams.farPlane = m_FarPlane;
    m_RenderParams.animator = m_Animator.get();
}

void FPSGame::OnUpdate(float deltaTime) {
    m_Time += deltaTime;

    // 简单旋转平行光
    m_Scene.dirLight.direction = glm::vec3(std::sin(0.3f * m_Time), -1.0f, std::cos(0.3f * m_Time));

    // 动画更新
    for (Animator* anim : m_Scene.animators) {
        if (anim) {
            anim->UpdateAnimation(deltaTime);
        }
    }

    // 处理键盘输入驱动相机
    auto& engine = GetEngine();
    if (engine.IsKeyPressed(GLFW_KEY_W))
        m_Camera.Move(HORIZON_FORWARD, deltaTime);
    if (engine.IsKeyPressed(GLFW_KEY_S))
        m_Camera.Move(HORIZON_BACKWARD, deltaTime);
    if (engine.IsKeyPressed(GLFW_KEY_A))
        m_Camera.Move(LEFT, deltaTime);
    if (engine.IsKeyPressed(GLFW_KEY_D))
        m_Camera.Move(RIGHT, deltaTime);
    if (engine.IsKeyPressed(GLFW_KEY_SPACE))
        m_Camera.Move(WORLD_UP, deltaTime);
    if (engine.IsKeyPressed(GLFW_KEY_LEFT_CONTROL))
        m_Camera.Move(WORLD_DOWN, deltaTime);
}

void FPSGame::OnRender() {
    auto& engine = GetEngine();

    // 每帧更新渲染参数
    m_RenderParams.width = engine.GetWidth();
    m_RenderParams.height = engine.GetHeight();
    m_RenderParams.renderMode = m_RenderMode;
    m_RenderParams.gammaCorrection = m_GammaCorrection;
    m_RenderParams.renderSkybox = m_RenderSkybox;
    m_RenderParams.orthoSize = m_OrthoSize;
    m_RenderParams.nearPlane = m_NearPlane;
    m_RenderParams.farPlane = m_FarPlane;
    m_RenderParams.spotLightOn = engine.IsSpotLightOn();
    m_RenderParams.animator = m_Animator.get();

    engine.RenderFrame(m_Scene, m_Camera, m_RenderParams);
}

void FPSGame::OnGui() {
    GUIContext ctx;
    ctx.camera = &m_Camera;
    ctx.renderMode = &m_RenderMode;
    ctx.gammaCorrection = &m_GammaCorrection;
    ctx.renderSkybox = &m_RenderSkybox;
    ctx.dirLight = &m_Scene.dirLight;
    ctx.orthoSize = &m_OrthoSize;
    ctx.nearPlane = &m_NearPlane;
    ctx.farPlane = &m_FarPlane;
    ctx.model = m_Model.get();
    ctx.animator = m_Animator.get();
    ctx.currentAnimationName = (m_Model && m_Animator) ? &m_CurrentAnimationName : nullptr;

    GetEngine().RenderGui(ctx);
}
