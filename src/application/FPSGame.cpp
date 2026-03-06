#include "application/FPSGame.h"

#include "world/scene.h"
#include "model/model_gltf.h"
#include "animation/animator.h"

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

#define RESOURCES_PATH(path) (std::string(RESOURCES_DIR) + path)

void FPSGame::OnInit() {
    // 初始化玩家
    m_Player = std::make_shared<Player>("MainPlayer");
    m_Player->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));

    // 初始化武器
    auto prisonModel = std::make_shared<Model>(RESOURCES_PATH("/environments/prison.glb"), true);
    shared_ptr<Entity> m_Prison = std::make_shared<Entity>("Prison");
    m_Prison->model = prisonModel;

    auto weaponModel = std::make_shared<Model>(RESOURCES_PATH("/models/ak74m.glb"), true);
    m_Player->weapon = std::make_shared<Weapon>("AK74");
    m_Player->weapon->model = weaponModel;

    // 加载动画
    if (!weaponModel->animations.empty()) {
        m_Player->weapon->animator = std::make_shared<Animator>(&weaponModel->animations.begin()->second);
        *m_Player->weapon->GetCurrentAnimationName() = weaponModel->animations.begin()->first;
    }

    // 初始化场景
    m_Scene.AddEntity(m_Player);
    m_Scene.AddEntity(m_Player->weapon);
    m_Scene.AddEntity(m_Prison);

    // 初始化渲染参数
    m_RenderParams.width = GetEngine().GetWidth();
    m_RenderParams.height = GetEngine().GetHeight();
    m_RenderParams.shadowWidth = GetEngine().GetShadowWidth();
    m_RenderParams.shadowHeight = GetEngine().GetShadowHeight();
    m_RenderParams.renderMode = m_RenderMode;
    m_RenderParams.gammaCorrection = m_GammaCorrection;
    m_RenderParams.renderSkybox = m_RenderSkybox;
    m_RenderParams.orthoSize = m_OrthoSize;
    m_RenderParams.nearPlane = m_NearPlane;
    m_RenderParams.farPlane = m_FarPlane;
    m_RenderParams.animator = m_Player->weapon->animator.get();
}

void FPSGame::OnUpdate(float deltaTime) {
    m_Time += deltaTime;    // 时间计数器

    ProcessMouseInput(deltaTime);
    ProcessKeyboardInput(deltaTime);

    // 更新场景
    m_Scene.Update(deltaTime);

    // 简单旋转平行光
    m_Scene.dirLight.direction = glm::vec3(std::sin(0.02f * m_Time), -1.0f, std::cos(0.1f * m_Time));
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
    m_RenderParams.spotLightOn = m_SpotLightOn;
    m_RenderParams.hideMouse = m_HideMouse;
    m_RenderParams.animator = m_Player->weapon->animator.get();

    engine.RenderFrame(m_Scene, *m_Player->camera, m_RenderParams);
}

void FPSGame::OnGui() {
    GUIContext ctx;
    ctx.camera = m_Player->camera.get();
    ctx.renderMode = &m_RenderMode;
    ctx.gammaCorrection = &m_GammaCorrection;
    ctx.renderSkybox = &m_RenderSkybox;
    ctx.dirLight = &m_Scene.dirLight;
    ctx.orthoSize = &m_OrthoSize;
    ctx.nearPlane = &m_NearPlane;
    ctx.farPlane = &m_FarPlane;
    ctx.model = m_Player->weapon->model.get();
    ctx.animator = m_Player->weapon->animator.get();
    ctx.currentAnimationName = m_Player->weapon->GetCurrentAnimationName();
    ctx.weaponOffset = &m_Player->weaponOffset;

    GetEngine().RenderGui(ctx);
}

void FPSGame::ProcessMouseInput(float deltaTime) {
    auto& engine = GetEngine();
    if (m_HideMouse) {
        float dx, dy;
        engine.GetMouseDelta(dx, dy);
        m_Player->HandleLook(dx * m_MouseSensitivity, dy * m_MouseSensitivity); 
    }
    if (engine.IsMousePressed(GLFW_MOUSE_BUTTON_LEFT)) {
        m_Player->weapon->Fire(deltaTime);
    }
}

void FPSGame::ProcessKeyboardInput(float deltaTime) {
    auto& engine = GetEngine();
    if (engine.IsKeyPressed(GLFW_KEY_W)) m_Player->HandleMove(HORIZON_FORWARD, deltaTime);          // W: 前进
    if (engine.IsKeyPressed(GLFW_KEY_S)) m_Player->HandleMove(HORIZON_BACKWARD, deltaTime);         // S: 后退
    if (engine.IsKeyPressed(GLFW_KEY_A)) m_Player->HandleMove(LEFT, deltaTime);                     // A: 左
    if (engine.IsKeyPressed(GLFW_KEY_D)) m_Player->HandleMove(RIGHT, deltaTime);                    // D: 右
    if (engine.IsKeyPressed(GLFW_KEY_SPACE)) m_Player->HandleMove(WORLD_UP, deltaTime);             // SPACE: 上
    if (engine.IsKeyPressed(GLFW_KEY_LEFT_CONTROL)) m_Player->HandleMove(WORLD_DOWN, deltaTime);    // CTRL: 下
    if (engine.IsKeyPressed(GLFW_KEY_R)) m_Player->weapon->Reload(deltaTime);                       // R: 换弹
    if (engine.IsKeyPressed(GLFW_KEY_Q)) m_Player->weapon->Draw(deltaTime);                         // Q: 切枪

    static bool shiftPressed = false;
    if (engine.IsKeyPressed(GLFW_KEY_LEFT_SHIFT)) {     // SHIFT: 切换鼠标隐藏
        if (!shiftPressed) {
            m_HideMouse = !m_HideMouse;
            engine.SetMouseHidden(m_HideMouse);
            shiftPressed = true;
        }
    } else {
        shiftPressed = false;
    }

    static bool fPressed = false;
    if (engine.IsKeyPressed(GLFW_KEY_F)) {              // F: 开关手电
        if (!fPressed) {
            m_SpotLightOn = !m_SpotLightOn;
            fPressed = true;
        }
    } else {
        fPressed = false;
    }
}