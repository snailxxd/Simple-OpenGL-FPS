#include "world/scene.h"
#include "world/entity.h"
#include "animation/animator.h"

Scene::Scene() {
    this->m_Entities = std::vector<std::shared_ptr<Entity>>();
}

Scene::~Scene() {}

void Scene::AddEntity(std::shared_ptr<Entity> entity) {
    m_Entities.push_back(entity);
}

void Scene::RemoveEntity(const std::string& name) {
    for (auto it = m_Entities.begin(); it != m_Entities.end(); ++it) {
        if ((*it)->GetName() == name) {
            m_Entities.erase(it);
            break;
        }
    }
}

void Scene::Update(float deltaTime) {
    for (auto entity : m_Entities) {
        entity->OnUpdate(deltaTime);
        if (entity->animator) {
            entity->animator->UpdateAnimation(deltaTime);
        }
    }
}
