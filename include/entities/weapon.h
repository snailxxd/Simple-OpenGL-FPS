#pragma once

#include "world/entity.h"
#include "model/model_gltf.h"
#include "animation/animator.h"

enum class WeaponState {
    IDLE,       // 待机
    FIRING,     // 开火
    RELOADING,  // 换弹
    HOLSTERED,  // 收刀/放回兜里
    DRAWING,    // 打开枪
};

class Weapon : public Entity {
public:
    WeaponState state = WeaponState::IDLE;

    Weapon(const std::string& name) : Entity(name) {
        state = WeaponState::IDLE;
    }

    void Fire(float deltaTime) {
        if (state == WeaponState::IDLE) {
            state = WeaponState::FIRING;
            animator->PlayAnimation(model->GetAnimation("Rig|Rig|AK_Shot"), false);
        }
    }

    void Reload(float deltaTime) {
        if (state == WeaponState::IDLE) {
            state = WeaponState::RELOADING;
            animator->PlayAnimation(model->GetAnimation("Rig|Rig|AK_Reload_full"), false);
        }
    }

    void Draw(float deltaTime) {
        if (state == WeaponState::IDLE) {
            state = WeaponState::DRAWING;
            animator->PlayAnimation(model->GetAnimation("Rig|Rig|AK_Draw"), false);
        }
    }

    void OnUpdate(float deltaTime) override {
        if (animator->IsFinished()) {
            state = WeaponState::IDLE;
            animator->PlayAnimation(model->GetAnimation("Rig|Rig|AK_Idle"), true);
        }
    }

};