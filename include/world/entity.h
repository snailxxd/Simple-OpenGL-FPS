#pragma once

#include <string>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Model;
class Animator;

/**
 * @class Entity
 * @brief 基础实体类
 */
class Entity {
public:
    std::shared_ptr<Model> model = nullptr;
    std::shared_ptr<Animator> animator = nullptr;

    Entity(const std::string& name) : m_Name(name) {}
    Entity(const std::string& name, std::shared_ptr<Model> model, std::shared_ptr<Animator> animator) : m_Name(name), model(model), animator(animator) {}
    virtual ~Entity() = default;

    glm::mat4 GetModelMatrix() {
        if (m_Dirty) {
            m_ModelMatrixCache = glm::mat4(1.0f);
            m_ModelMatrixCache = glm::translate(m_ModelMatrixCache, position);
            m_ModelMatrixCache = glm::rotate(m_ModelMatrixCache, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));    // Yaw
            m_ModelMatrixCache = glm::rotate(m_ModelMatrixCache, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));    // Pitch
            m_ModelMatrixCache = glm::rotate(m_ModelMatrixCache, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));    // Roll
            m_ModelMatrixCache = glm::scale(m_ModelMatrixCache, scale);
            m_Dirty = false;
        }
        return m_ModelMatrixCache;
    }

    virtual void OnUpdate(float deltaTime) {}

    void SetPosition(const glm::vec3& pos) { position = pos; m_Dirty = true; }
    void SetRotation(const glm::vec3& rot) { rotation = rot; m_Dirty = true; }
    void SetScale(const glm::vec3& scale) { this->scale = scale; m_Dirty = true; }

    const std::string& GetName() const { return m_Name; }
    std::string* GetCurrentAnimationName() { return &m_CurrentAnimationName; }
    

protected:
    std::string m_Name;
    std::string m_CurrentAnimationName;

    glm::vec3 position{0.0f};       // 位置
    glm::vec3 rotation{0.0f};       // 旋转 (x: pitch, y: yaw, z: roll) 单位为角度
    glm::vec3 scale{1.0f};          // 缩放

    bool m_Dirty = true;            // 标记是否需要重新计算模型矩阵
    glm::mat4 m_ModelMatrixCache;   // 缓存上一帧的模型矩阵
};