#include "render_engine/scene.h"

Scene::Scene() {
    this->models.clear();
    this->animators.clear();
}

Scene::Scene(std::vector<Model*> models, std::vector<Animator*> animators) {
    this->models = models;
    this->animators = animators;
}

void Scene::Update(float deltaTime) {
    for (Animator* animator : animators) {
        if (animator) {
            animator->UpdateAnimation(deltaTime);
        }
    }
}

