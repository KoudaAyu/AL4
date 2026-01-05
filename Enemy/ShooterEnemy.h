#pragma once

#include "Enemy.h"
#include "KamataEngine.h"
#include "../AABB.h"
#include <list>

class Player;
class MapChipField;

class ShooterEnemy : public Enemy {
public:
    ShooterEnemy() = default;
    ~ShooterEnemy() override;

    void Initialize(KamataEngine::Camera* camera, const KamataEngine::Vector3& pos);
    void Update() override;
    void Draw() override;

    // configuration
    void SetFireInterval(float seconds) { fireInterval_ = seconds; }
    void SetBulletSpeed(float s) { bulletSpeed_ = s; }

    // control whether this shooter instance is allowed to fire (used to disable shooting during countdown)
    void SetAllowShooting(bool allow) { allowShooting_ = allow; }

    // control facing direction: if true bullets go to the right, otherwise to the left
    void SetFacingRight(bool right);
    bool IsFacingRight() const { return faceRight_; }

    // Remove the first bullet found colliding with the given AABB. Returns true if a bullet was consumed.
    bool ConsumeBulletCollidingWithAABB(const AABB& aabb);

    // Cull bullets that hit map blocks (mark them not alive)
    void CullBulletsByMap(MapChipField* map);

private:
    struct Bullet {
        KamataEngine::Vector3 pos{0,0,0};
        KamataEngine::Vector3 vel{0,0,0};
        bool alive = false;
        KamataEngine::WorldTransform wt;
        KamataEngine::Model* model = nullptr;
        bool ownsModel = false;
        AABB aabb;
        Bullet() {
            wt.Initialize();
        }
    };

    float timer_ = 0.0f;
    float fireInterval_ = 2.0f; // seconds
    float bulletSpeed_ = 1.0f;

    std::list<Bullet*> bullets_;

    // shooter model file handles
    uint32_t soundHandle_ = 0u;

    // whether this shooter instance is allowed to spawn bullets
    bool allowShooting_ = false;

    // whether shooter faces right (true) or left (false). Controls bullet direction.
    bool faceRight_ = false;

    // optional secondary model for the shooter's equipment (like shield/gun)
    KamataEngine::Model* shooterModel_ = nullptr;
    bool ownsShooterModel_ = false;
    KamataEngine::WorldTransform shooterWorldTransform_;

    // shared model for bullets (prefer using EnemyBullet for shooter bullets)
    KamataEngine::Model* bulletModel_ = nullptr;
    bool ownsBulletModel_ = false;
};
