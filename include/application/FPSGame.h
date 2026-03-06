#pragma once

#include "player/player.h"
#include "application/Application.h"
#include "world/scene.h"
#include "render_engine/renderer.h"
#include "render_engine/gui_manager.h"

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
    Scene m_Scene;                      // 场景
    std::shared_ptr<Player> m_Player;   // 玩家

    // 渲染参数与 GUI 控制变量
    RenderParams m_RenderParams{};
    int m_RenderMode = 0;
    int m_GammaCorrection = 0;
    int m_RenderSkybox = 0;
    float m_OrthoSize = 5.0f;
    float m_NearPlane = 1.0f;
    float m_FarPlane = 20.0f;
    bool m_SpotLightOn = false;
    bool m_HideMouse = true;
    float m_MouseSensitivity = 0.05f;

    // 简单时间累计，用于灯光动画
    float m_Time = 0.0f;

    void ProcessKeyboardInput(float deltaTime);
    void ProcessMouseInput(float deltaTime);
};
