#pragma once

#include "render_engine/IRenderEngine.h"

/**
 * @class Application
 * @brief 应用抽象基类，与具体渲染后端解耦，由 Run() 驱动主循环
 */
class Application {
public:
    explicit Application(IRenderEngine& engine);
    virtual ~Application() = default;

    void Run();

protected:
    IRenderEngine& GetEngine() { return m_Engine; }
    IRenderEngine const& GetEngine() const { return m_Engine; }

    /// 应用初始化
    virtual void OnInit() {}

    /// 每帧更新
    virtual void OnUpdate(float deltaTime) { (void)deltaTime; }

    /// 每帧渲染
    virtual void OnRender() {}

    /// 每帧 GUI 渲染前调用，可在此准备或触发 GUI 逻辑
    virtual void OnGui() {}

private:
    IRenderEngine& m_Engine;
};
