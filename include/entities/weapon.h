#pragma once

#include "world/entity.h"

enum class WeaponState {
    IDLE,       // 待机
    FIRING,     // 开火
    RELOADING,  // 换弹
    HOLSTERED,  // 收刀/放回兜里
    AIMING      // 瞄准
};

class Weapon : public Entity {
public:
    Weapon(const std::string& name) : Entity(name) {}
};