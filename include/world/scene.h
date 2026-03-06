#pragma once

#include <vector>
#include <memory>

#include "light/light_pbr.h"
#include "entity.h"

struct Scene {
public:
    Scene();
    ~Scene();

    void AddEntity(std::shared_ptr<Entity> entity);
    void RemoveEntity(const std::string& name);

    void Update(float deltaTime);

    const std::vector<std::shared_ptr<Entity>>& GetEntities() const { return m_Entities; }
    const int GetEntityCount() const { return static_cast<int>(m_Entities.size()); }

    // 光照
    DirLight dirLight{glm::vec3(0.0f, -1.0f, -0.5f), glm::vec3(1.0f), 5.0f};
    SpotLight spotLight{glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(1.0f), 10.0f};

private:
    std::vector<std::shared_ptr<Entity>> m_Entities;
};

