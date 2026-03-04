#pragma once

#include <vector>

#include "light/light_pbr.h"
#include "model/model_gltf.h"
#include "animation/animator.h"

struct Scene {
public:
    // 模型与动画
    std::vector<Model*> models;
    std::vector<Animator*> animators;

    // 光照
    DirLight dirLight{glm::vec3(0.0f, -1.0f, -0.5f), glm::vec3(1.0f), 5.0f};
    SpotLight spotLight{glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(1.0f), 7.0f};

    Scene();
    Scene(std::vector<Model*> models, std::vector<Animator*> animators);

    void Update(float deltaTime);
};

