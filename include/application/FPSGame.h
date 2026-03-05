#pragma once

#include "application/Application.h"
#include "render_engine/scene.h"
#include "render_engine/renderer.h"
#include "render_engine/gui_manager.h"
#include "camera/camera.h"

#include <memory>
#include <string>

/**
 * @brief FPS 游戏应用：在应用层持有并驱动场景/相机/模型/动画
 */
class FPSGame : public Application {
public:
    using Application::Application;

protected:
    void OnInit() override;
    void OnUpdate(float deltaTime) override;
    void OnRender() override;
    void OnGui() override;

private:
    // 场景与相机由应用层拥有
    Scene m_Scene;
    Camera m_Camera{
        glm::vec3(0.0f, 0.0f, 3.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        -90.0f,
        0.0f
    };

    // 模型与动画
    std::unique_ptr<class Model> m_Model;
    std::unique_ptr<class Animator> m_Animator;
    std::string m_CurrentAnimationName;

    // 渲染参数与 GUI 控制变量
    RenderParams m_RenderParams{};
    int m_RenderMode = 0;
    int m_GammaCorrection = 0;
    int m_RenderSkybox = 0;
    float m_OrthoSize = 2.5f;
    float m_NearPlane = 1.0f;
    float m_FarPlane = 20.0f;

    // 简单时间累计，用于灯光动画
    float m_Time = 0.0f;
};
