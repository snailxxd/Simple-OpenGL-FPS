#pragma once

#include <memory>
#include "camera/camera.h"
#include "entities/weapon.h"

enum class PlayerMovementState {
    IDLE,       // 待机
    WALKING,    // 走路
    JUMPING,    // 跳跃/在空中
    CROUCHING   // 蹲下
};

class Player : public Entity {
public:
    std::shared_ptr<Camera> camera;
    std::shared_ptr<Weapon> weapon;

    PlayerMovementState state = PlayerMovementState::IDLE;
    
    Animation* walkingAnimation = nullptr;
    Animation* idleAnimation = nullptr;

    float eyeHeight = 1.7f;                     // 摄像机高度
    glm::vec3 weaponOffset{0.20f, -0.26f, 0.29f};   // 武器偏移量

    Player(const std::string& name) : Entity(name) {
        camera = std::make_shared<Camera>(this->position + glm::vec3(0.0f, eyeHeight, 0.0f));
    }

    void HandleMove(Camera_Movement direction, float deltaTime) {
        camera->Move(direction, deltaTime);
        this->position = camera->Position - glm::vec3(0, eyeHeight, 0); 
        this->m_Dirty = true;
        state = PlayerMovementState::WALKING;
    }

    void HandleLook(float xoffset, float yoffset) {
        camera->PitchAndYaw(xoffset, yoffset);
    }

    void OnUpdate(float dt) override {
         if (weapon && camera) {
            glm::vec3 targetWeaponPos = camera->Position 
                                      + camera->Right * weaponOffset.x 
                                      + camera->Up * weaponOffset.y 
                                      + camera->Front * weaponOffset.z;
            weapon->SetPosition(targetWeaponPos);
            weapon->SetRotation(glm::vec3(-camera->Pitch, -camera->Yaw + 90.0f, 0.0f));
        }

        HandleAnimations();
        state = PlayerMovementState::IDLE;
    }

private:
    void HandleAnimations() {
        if (!weapon || !weapon->animator || !weapon->model) return;

        if (!walkingAnimation) walkingAnimation = weapon->model->GetAnimation("Rig|Rig|AK_Walk");
        if (!idleAnimation)    idleAnimation    = weapon->model->GetAnimation("Rig|Rig|AK_Idle");

        if (weapon->state == WeaponState::IDLE) {
            Animation* targetAnim = (state == PlayerMovementState::WALKING) ? walkingAnimation : idleAnimation;
            if (weapon->animator->GetCurrentAnimation() != targetAnim) {
                weapon->animator->PlayAnimation(targetAnim, true);
            }
        }
    }
};