#pragma once

struct GLFWwindow;

class GUIManager {
public:
    GUIManager(GLFWwindow* window);
    ~GUIManager();

    void init_gui();
    void render_gui();
    void shutdown_gui();

private:
    GLFWwindow* m_Window;
};

