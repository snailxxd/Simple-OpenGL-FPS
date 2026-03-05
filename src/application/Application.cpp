#include "application/Application.h"

Application::Application(IRenderEngine& engine)
    : m_Engine(engine) {
}

void Application::Run() {
    if (!m_Engine.Initialize()) {
        return;
    }

    OnInit();

    while (!m_Engine.ShouldClose()) {
        m_Engine.BeginFrame();
        OnUpdate(m_Engine.GetDeltaTime());
        OnRender();
        OnGui();
        m_Engine.EndFrame();
    }
}
