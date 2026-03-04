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

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#define RESOURCES_PATH(path) (std::string(RESOURCES_DIR) + path)

struct SkyboxData {
    unsigned int vao;
    unsigned int cubemapID;
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

void processInput(GLFWwindow *window);

unsigned int init_shadow_fbo(unsigned int &depthMap);

glm::mat4 get_light_space_matrix(glm::vec3 lightDir);

void render_scene(Shader& shader);

SkyboxData loadHDRtoSkybox(const std::string &hdrPath);

unsigned int init_screenVAO();
unsigned int init_framebuffer(unsigned int &textureColorBuffer, unsigned int &textureDepthBuffer);
void render_screen(Shader& screenShader, unsigned int screenVAO, unsigned int texColorBuffer);

void init_gui(GLFWwindow* window);
void render_gui(GLFWwindow* window);

// 窗口尺寸
int WINDOW_WIDTH = 1920;
int WINDOW_HEIGHT = 1080;

// 时间相关
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// 鼠标控制相关
bool hideMouse = true;
float lastX = WINDOW_WIDTH / 2.0f;
float lastY = WINDOW_HEIGHT / 2.0f;
bool firstMouse = true;
float sensitivity = 0.05f;

int renderMode = 0;         // 0: Fill, 1: Wireframe
int renderSkybox = 0;       // 0: 不渲染天空盒, 1: 渲染天空盒
bool spotLightOn = false;   // 手电开关
int gammaCorrection = 0;    // 0: 伽马矫正, 1: 不执行伽马矫正

// 光照
DirLight dirlight = DirLight(glm::vec3(0.0f, -1.0f, -0.5f), glm::vec3(1.0f), 5.0f);
SpotLight spotLight = SpotLight(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(1.0f), 7.0f);

// 阴影贴图
int SHADOW_WIDTH = 2048;
int SHADOW_HEIGHT = 2048;

float orthoSize = 2.5f;
float nearPlane = 1.0f;
float farPlane = 20.0f;

// 模型
Model* ourModel;
// 动画播放器
Animator* animator;

std::map<std::string, Animation>::iterator currentAnimation;

// 背景颜色
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

    // 加载并编译着色器
    Shader shaderProgram(RESOURCES_PATH("/shaders/core/pbr.vs"), RESOURCES_PATH("/shaders/core/pbr.fs"));
    Shader screenShader(RESOURCES_PATH("/shaders/frame/frame.vs"), RESOURCES_PATH("/shaders/frame/frame.fs"));
    Shader skyboxShader(RESOURCES_PATH("/shaders/skybox/skybox.vs"), RESOURCES_PATH("/shaders/skybox/skybox.fs"));
    Shader shadowShader(RESOURCES_PATH("/shaders/core/shadow.vs"), RESOURCES_PATH("/shaders/core/shadow.fs"));

    // 加载天空盒
    SkyboxData skyboxData = loadHDRtoSkybox(RESOURCES_PATH("/hdr/citrus_orchard_puresky_1k.hdr"));

    // 加载模型
    ourModel = new Model(RESOURCES_PATH("/models/fps_ak-74m_animations.glb"), true);
    cout << "Model loaded: " << ourModel->meshes.size() << " meshes, " 
         << ourModel->textures_loaded.size() << " textures, " 
         << ourModel->GetBoneCounter() << " bones" << endl;
    if (ourModel->animations.size() > 0) {
        animator = new Animator(&ourModel->animations.begin()->second);
        currentAnimation = ourModel->animations.begin();
    }

    init_gui(window);   // 初始化 GUI

    // 初始化摄像机
    Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f);
    glfwSetWindowUserPointer(window, &camera);

    // 初始化帧缓冲
    unsigned int texColorBuffer;
    unsigned int texDepthBuffer;
    unsigned int screenVAO = init_screenVAO();
    unsigned int FBO = init_framebuffer(texColorBuffer, texDepthBuffer);

    // 初始化阴影
    unsigned int depthMap;
    unsigned int shadow_fbo = init_shadow_fbo(depthMap);

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

        dirlight.direction = glm::vec3(sin(0.3*currentFrame),-1.0f, cos(0.3*currentFrame));   // 模拟日光转动

        if (renderMode)                 // 切换线框模式
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glEnable(GL_DEPTH_TEST);        // 启用深度测试


        // 渲染阴影贴图
        glm::mat4 lightSpaceMatrix = get_light_space_matrix(dirlight.direction);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo);
        glClear(GL_DEPTH_BUFFER_BIT);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);

        auto transforms = animator->GetFinalBoneMatrices();
        
        shadowShader.use();
        shadowShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

		for (int i = 0; i < transforms.size(); ++i) 
			shadowShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);

        render_scene(shadowShader);
        
        glCullFace(GL_BACK);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 渲染场景
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

        glBindFramebuffer(GL_FRAMEBUFFER, FBO);     // 绑定帧缓冲
        glClearColor(BG_COLOR.r, BG_COLOR.g, BG_COLOR.b, BG_COLOR.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 清屏并清除深度缓冲
        shaderProgram.use();

        shaderProgram.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        // 光照
        spotLight.position = camera.Position;
        spotLight.direction = camera.Front;
        dirlight.setUniform(shaderProgram);
        spotLight.setUniform(shaderProgram);
        if (!spotLightOn)
            spotLight.shut(shaderProgram);

        shaderProgram.setVec3("viewPos", camera.Position);

        // 设置变换矩阵
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = camera.GetProjectionMatrix((float)WINDOW_WIDTH / (float)WINDOW_HEIGHT);
        shaderProgram.setMat4("view", view);
        shaderProgram.setMat4("projection", projection);

		for (int i = 0; i < transforms.size(); ++i)
			shaderProgram.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);

        // 绑定阴影贴图
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        shaderProgram.setInt("dirLight.shadowMap", 6);

        render_scene(shaderProgram);

        if (renderSkybox) {
            glDepthFunc(GL_LEQUAL); 
            skyboxShader.use();
            
            glm::mat4 skyView = glm::mat4(glm::mat3(camera.GetViewMatrix())); 
            skyboxShader.setMat4("view", skyView);
            skyboxShader.setMat4("projection", projection);
            
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxData.cubemapID);
            skyboxShader.setInt("skybox", 0);

            glBindVertexArray(skyboxData.vao);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glDepthFunc(GL_LESS);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);   // 绑定回默认帧缓冲
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        if (!gammaCorrection)
            glEnable(GL_FRAMEBUFFER_SRGB);
        render_screen(screenShader, screenVAO, texColorBuffer); // 绘制到屏幕

        glDisable(GL_FRAMEBUFFER_SRGB);
        render_gui(window);     // 绘制 GUI

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

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

SkyboxData loadHDRtoSkybox(const std::string &hdrPath) {
    SkyboxData result;

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
    float *data = stbi_loadf(hdrPath.c_str(), &width, &height, &nrComponents, 0);
    unsigned int hdrTexture;
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
    Shader convertShader(RESOURCES_PATH("/shaders/skybox/equirect_to_cubemap.vs"), RESOURCES_PATH("/shaders/skybox/equirect_to_cubemap.fs"));
    convertShader.use();
    convertShader.setInt("equirectangularMap", 0);
    convertShader.setMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);

    glViewport(0, 0, 512, 512); 
    for (unsigned int i = 0; i < 6; ++i) {
        convertShader.setMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, result.cubemapID, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindVertexArray(result.vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // 清理临时资源
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteTextures(1, &hdrTexture);
    glDeleteFramebuffers(1, &captureFBO);
    glDeleteRenderbuffers(1, &captureRBO);
    // 恢复主视口
    extern int WINDOW_WIDTH, WINDOW_HEIGHT; 
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    return result;
};

void render_scene(Shader& shader) {
        shader.use();
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // 移动模型到合适位置
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));    // 旋转模型
        model = glm::scale(model, glm::vec3(1.0f));    // 缩放模型

        ourModel->Draw(shader, model);   // 渲染模型
}

glm::mat4 get_light_space_matrix(glm::vec3 lightDir) {
    glm::mat4 lightProjection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, nearPlane, farPlane);
    glm::mat4 lightView = glm::lookAt(lightDir * -10.0f, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    return lightProjection * lightView;
}

void render_skybox(Shader& skyboxShader, glm::mat4 view, glm::mat4 projection, unsigned int skyboxVAO, unsigned int cubemapTexture) {
    glDepthFunc(GL_LEQUAL);
    skyboxShader.use();
    view = glm::mat4(glm::mat3(view));
    skyboxShader.setMat4("view", view);
    skyboxShader.setMat4("projection", projection);
    glBindVertexArray(skyboxVAO);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthFunc(GL_LESS);
}

unsigned int init_framebuffer(unsigned int &texColorBuffer, unsigned int &texDepthBuffer) {
    // 初始化帧缓冲
    unsigned int FBO;
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    // 颜色缓冲纹理
    glGenTextures(1, &texColorBuffer);
    glBindTexture(GL_TEXTURE_2D, texColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);

    // 深度缓冲纹理
    glGenTextures(1, &texDepthBuffer);
    glBindTexture(GL_TEXTURE_2D, texDepthBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texDepthBuffer, 0);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return FBO;
}

void render_screen(Shader& screenShader, unsigned int screenVAO, unsigned int texColorBuffer) {
    screenShader.use();
    glBindVertexArray(screenVAO);
    glBindTexture(GL_TEXTURE_2D, texColorBuffer);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

unsigned int init_screenVAO() {
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

unsigned int init_shadow_fbo(unsigned int &depthMap) {
    // 初始化帧缓冲
    unsigned int FBO;
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    // 深度缓冲纹理
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };   // 边界外无阴影
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return FBO;
}

void init_gui(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true); 
    ImGui_ImplOpenGL3_Init("#version 330 core");
}

void render_gui(GLFWwindow* window) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Control Panel");    // 窗口标题

    float fps = ImGui::GetIO().Framerate;
    ImGui::Text("FPS: %.0f", fps);      // 显示 FPS

    // 摄像机设置
    ImGui::BeginChild("Camera Controls", ImVec2(0, 80), true);
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Camera Settings");
    Camera& camera = *(Camera*)glfwGetWindowUserPointer(window);
    ImGui::SliderFloat("FOV", &camera.Fov, 30.0f, 120.0f, "%.0f", ImGuiSliderFlags_AlwaysClamp);  // fov 控制滑块
    ImGui::SliderFloat("Camera Speed", &camera.MovementSpeed, 0.1f, 10.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp); // 摄像机速度滑块
    ImGui::EndChild();

    ImGui::BeginChild("Settings", ImVec2(0, 140), true);
    ImGui::Text("Render Mode:");
    ImGui::RadioButton("Fill", &renderMode, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Wireframe", &renderMode, 1);

    ImGui::Text("Gamma Correction");
    ImGui::RadioButton("On", &gammaCorrection, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Off", &gammaCorrection, 1);

    ImGui::Text("Render Skybox:");
    ImGui::RadioButton("No", &renderSkybox, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Yes", &renderSkybox, 1);

    ImGui::EndChild();

    ImGui::BeginChild("Light Controls", ImVec2(0, 100), true);
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Light Controls");

    ImGui::ColorEdit3("Light Color", (float*)&dirlight.color);
    ImGui::SliderFloat("Light Intensity", &dirlight.intensity, 0.0f, 10.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::EndChild();

    ImGui::BeginChild("Shadow Controls", ImVec2(0, 100), true);
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Shadow Controls");

    ImGui::SliderFloat("Ortho Size", &orthoSize, 0.0f, 15.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::SliderFloat("Near Plane", &nearPlane, 0.0f, 20.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::SliderFloat("Far Plane", &farPlane, 0.0f, 20.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::EndChild();

    ImGui::BeginChild("Animation Controls", ImVec2(0, 80), true);
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Animation Controls");
    if (ImGui::BeginCombo("Animation", currentAnimation->first.c_str())) {
        for (auto it = ourModel->animations.begin(); it != ourModel->animations.end(); ++it) {
            bool isSelected = (it == currentAnimation);
            if (ImGui::Selectable(it->first.c_str(), isSelected)) {
                currentAnimation = it;
                animator->PlayAnimation(&currentAnimation->second);
            }
        }
        ImGui::EndCombo();
    }
    ImGui::SliderFloat("Animation Speed", &animator->GetSpeed(), 0.1f, 3.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::EndChild();

    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}