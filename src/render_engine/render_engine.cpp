#include <iostream>
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shader/shader.h"
#include "camera/camera.h"
#include "light/light_pbr.h"
#include "model/model_gltf.h"
#include "animation/animation.h"
#include "animation/animator.h"
#include <stb_image.h>
#include "render_engine/gui_manager.h"
#include "render_engine/engine_globals.h"
#include "render_engine/scene.h"
#include "render_engine/renderer.h"

#define RESOURCES_PATH(path) (std::string(RESOURCES_DIR) + path)

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

void processInput(GLFWwindow *window);

// 引擎全局状态
int WINDOW_WIDTH = 1920;
int WINDOW_HEIGHT = 1080;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool hideMouse = true;
float lastX = WINDOW_WIDTH / 2.0f;
float lastY = WINDOW_HEIGHT / 2.0f;
bool firstMouse = true;
float sensitivity = 0.05f;

int renderMode = 0;
int renderSkybox = 0;
bool spotLightOn = false;
int gammaCorrection = 0;

DirLight dirlight = DirLight(glm::vec3(0.0f, -1.0f, -0.5f), glm::vec3(1.0f), 5.0f);
SpotLight spotLight = SpotLight(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(1.0f), 7.0f);

int SHADOW_WIDTH = 2048;
int SHADOW_HEIGHT = 2048;

float orthoSize = 2.5f;
float nearPlane = 1.0f;
float farPlane = 20.0f;

Model* ourModel = nullptr;
Animator* animator = nullptr;

std::map<std::string, Animation>::iterator currentAnimation;

glm::vec4 BG_COLOR(0.02f, 0.02f, 0.02f, 1.0f);

int main() {
    // 初始化窗口
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口并绑定上下文
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "MyRenderEngine", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // 注册回调函数
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetKeyCallback(window, key_callback);

    // 初始化 GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // 创建渲染器
    Renderer renderer;

    // 创建 GUI 管理器
    GUIManager guiManager(window);

    // 加载模型
    ourModel = new Model(RESOURCES_PATH("/models/fps_ak-74m_animations.glb"), true);
    cout << "Model loaded: " << ourModel->meshes.size() << " meshes, " 
         << ourModel->textures_loaded.size() << " textures, " 
         << ourModel->GetBoneCounter() << " bones" << endl;
    if (ourModel->animations.size() > 0) {
        animator = new Animator(&ourModel->animations.begin()->second);
        currentAnimation = ourModel->animations.begin();
    }

    // 初始化摄像机
    Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f);
    glfwSetWindowUserPointer(window, &camera);

    // 初始化场景
    Scene scene;
    scene.models.push_back(ourModel);
    if (animator) {
        scene.animators.push_back(animator);
    }
    scene.dirLight = dirlight;
    scene.spotLight = spotLight;

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);    // 隐藏鼠标
    glEnable(GL_CULL_FACE);         // 启用面剔除

    // 渲染循环
    while (!glfwWindowShouldClose(window)) {
        // 时间计算逻辑
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);   // 处理输入
        animator->UpdateAnimation(deltaTime);

        // 更新光源方向并同步到场景
        dirlight.direction = glm::vec3(sin(0.3f * currentFrame), -1.0f, cos(0.3f * currentFrame));
        scene.dirLight = dirlight;
        scene.spotLight = spotLight;

        renderer.BeginFrame();
        renderer.RenderShadows(scene);          // 渲染阴影贴图
        renderer.RenderScene(scene, camera);    // 渲染场景
        renderer.RenderSkybox(scene, camera);   // 渲染天空盒
        renderer.EndFrame();

        guiManager.render_gui();                // 渲染 GUI

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window) {
    Camera& camera = *(Camera*)glfwGetWindowUserPointer(window);
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)  // 按 ESC 退出
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)       // 按 W 前进
        camera.Move(HORIZON_FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)       // 按 S 后退
        camera.Move(HORIZON_BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)       // 按 A 左移
        camera.Move(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)       // 按 D 右移
        camera.Move(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)    // 按 空格 上移
        camera.Move(WORLD_UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) // 按 左Ctrl 下移
        camera.Move(WORLD_DOWN, deltaTime);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    WINDOW_WIDTH = width;
    WINDOW_HEIGHT = height;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (!hideMouse) {
        firstMouse = true;
        return;
    }

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    Camera& camera = *(Camera*)glfwGetWindowUserPointer(window);
    camera.PitchAndYaw(xoffset * sensitivity, yoffset * sensitivity);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS) {
        hideMouse = !hideMouse;
        if (hideMouse) {
            firstMouse = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
    if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        spotLightOn = !spotLightOn;
    }
}