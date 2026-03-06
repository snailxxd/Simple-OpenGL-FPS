#pragma once

#include <string>
#include <glm/glm.hpp>

struct GLFWwindow;
class Camera;
class DirLight;
class Model;
class Animator;

/// GUI 所需数据与可写状态，由引擎传入，替代 engine_globals
struct GUIContext {
    Camera* camera = nullptr;
    int* renderMode = nullptr;
    int* gammaCorrection = nullptr;
    int* renderSkybox = nullptr;
    DirLight* dirLight = nullptr;
    float* orthoSize = nullptr;
    float* nearPlane = nullptr;
    float* farPlane = nullptr;
    Model* model = nullptr;
    Animator* animator = nullptr;
    std::string* currentAnimationName = nullptr;

    glm::vec3* weaponOffset = nullptr;
};

class GUIManager {
public:
    GUIManager(GLFWwindow* window);
    ~GUIManager();

    void init_gui();
    void render_gui(const GUIContext& ctx);
    void shutdown_gui();

private:
    GLFWwindow* m_Window;
};

