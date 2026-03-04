#pragma once

#include <map>
#include <glm/glm.hpp>

#include "light/light_pbr.h"
#include "model/model_gltf.h"
#include "animation/animation.h"
#include "animation/animator.h"

// 窗口尺寸
extern int WINDOW_WIDTH;
extern int WINDOW_HEIGHT;

// 时间相关
extern float deltaTime;
extern float lastFrame;

// 鼠标控制相关
extern bool hideMouse;
extern float lastX;
extern float lastY;
extern bool firstMouse;
extern float sensitivity;

// 渲染/显示相关设置
extern int renderMode;        // 0: Fill, 1: Wireframe
extern int renderSkybox;      // 0: 不渲染天空盒, 1: 渲染天空盒
extern bool spotLightOn;      // 手电开关
extern int gammaCorrection;   // 0: 伽马矫正, 1: 不执行伽马矫正

// 光照
extern DirLight dirlight;
extern SpotLight spotLight;

// 阴影贴图
extern int SHADOW_WIDTH;
extern int SHADOW_HEIGHT;

extern float orthoSize;
extern float nearPlane;
extern float farPlane;

// 模型与动画
extern Model* ourModel;
extern Animator* animator;
extern std::map<std::string, Animation>::iterator currentAnimation;

// 背景颜色
extern glm::vec4 BG_COLOR;

