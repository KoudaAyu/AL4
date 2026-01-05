#include "ShooterEnemy.h"
#include "KamataEngine.h"
#include "../MathUtl.h"
#include "../MapChipField.h"
#include <cmath>
#include <numbers>

using namespace KamataEngine;

ShooterEnemy::~ShooterEnemy() {
    for (auto b : bullets_) {
        if (!b) continue;
        if (b->ownsModel && b->model) {
            delete b->model;
            b->model = nullptr;
        }
        delete b;
    }
    bullets_.clear();

    if (ownsModel_ && model_) {
        delete model_;
        model_ = nullptr;
    }
}

void ShooterEnemy::Initialize(KamataEngine::Camera* camera, const KamataEngine::Vector3& pos) {
    model_ = Model::CreateFromOBJ("Enemy", true);
    ownsModel_ = (model_ != nullptr);
    camera_ = camera;

    worldTransform_.Initialize();
    worldTransform_.translation_ = pos;
    worldTransform_.rotation_ = {0, 0, 0};
    worldTransform_.scale_ = {0.9f, 0.9f, 0.9f};

    // ensure model faces according to current facing flag
    SetFacingRight(faceRight_);

    UpdateAABB();

    // allocate bullet pool
    for (int i = 0; i < 8; ++i) {
        Bullet* b = new Bullet();
        b->alive = false;
        // use shared model if possible
        if (model_) {
            b->model = model_;
            b->ownsModel = false;
        } else {
            b->model = KamataEngine::Model::CreateFromOBJ("Enemy", true);
            b->ownsModel = (b->model != nullptr);
        }
        bullets_.push_back(b);
    }
}

void ShooterEnemy::SetFacingRight(bool right) {
    faceRight_ = right;
    // Use same convention as Player: right = pi/2, left = 3*pi/2
    if (faceRight_) {
        worldTransform_.rotation_.y = static_cast<float>(std::numbers::pi / 2.0);
    } else {
        worldTransform_.rotation_.y = static_cast<float>(std::numbers::pi * 3.0 / 2.0);
    }
}

void ShooterEnemy::Update() {
    if (!isAlive_) return;

    worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
    worldTransform_.TransferMatrix();

    // Only advance firing timer when shooting is allowed
    if (allowShooting_) {
        timer_ += 1.0f / 60.0f;
    }

    if (timer_ >= fireInterval_) {
        // Only spawn bullets when allowed
        if (allowShooting_) {
            timer_ = 0.0f;
            // derive facing from actual model rotation to ensure visual and behavior match
            bool facingRight = (std::sin(worldTransform_.rotation_.y) > 0.0f);
            Vector3 dir = facingRight ? Vector3{1.0f, 0.0f, 0.0f} : Vector3{-1.0f, 0.0f, 0.0f};
            for (auto b : bullets_) {
                if (!b->alive) {
                    b->alive = true;
                    b->pos = worldTransform_.translation_;
                    b->pos.z = 0.0f;
                    b->vel = {dir.x * bulletSpeed_, dir.y * bulletSpeed_, 0.0f};
                    b->wt.translation_ = b->pos;
                    b->wt.scale_ = {0.3f, 0.3f, 0.3f};
                    // align bullet model rotation with shooter so it visually faces same direction
                    b->wt.rotation_ = worldTransform_.rotation_;
                    b->wt.matWorld_ = MakeAffineMatrix(b->wt.scale_, b->wt.rotation_, b->wt.translation_);
                    b->wt.TransferMatrix();
                    static constexpr float bw = 0.2f;
                    b->aabb.min = {b->pos.x - bw, b->pos.y - bw, -0.5f};
                    b->aabb.max = {b->pos.x + bw, b->pos.y + bw, 0.5f};
                    break;
                }
            }
        } else {
            // if not allowed, clamp timer so it doesn't overflow repeatedly
            timer_ = fireInterval_;
        }
    }

    for (auto b : bullets_) {
        if (!b->alive) continue;
        b->pos.x += b->vel.x;
        b->pos.y += b->vel.y;
        b->wt.translation_ = b->pos;
        b->wt.matWorld_ = MakeAffineMatrix(b->wt.scale_, b->wt.rotation_, b->wt.translation_);
        b->wt.TransferMatrix();
        // update AABB to current position
        static constexpr float bw = 0.2f;
        b->aabb.min = {b->pos.x - bw, b->pos.y - bw, -0.5f};
        b->aabb.max = {b->pos.x + bw, b->pos.y + bw, 0.5f};
        if (std::fabs(b->pos.x - worldTransform_.translation_.x) > 30.0f || std::fabs(b->pos.y - worldTransform_.translation_.y) > 30.0f) {
            b->alive = false;
        }
    }

    UpdateAABB();
}

void ShooterEnemy::Draw() {
    if (!isAlive_) return;
    if (model_ && camera_) model_->Draw(worldTransform_, *camera_);

    for (auto b : bullets_) {
        if (!b->alive) continue;
        if (b->model && camera_) b->model->Draw(b->wt, *camera_);
    }
}

bool ShooterEnemy::ConsumeBulletCollidingWithAABB(const AABB& aabb) {
    for (auto b : bullets_) {
        if (!b->alive) continue;
        if (IsCollisionAABBAABB(b->aabb, aabb)) {
            b->alive = false;
            return true;
        }
    }
    return false;
}

void ShooterEnemy::CullBulletsByMap(MapChipField* map) {
    if (!map) return;
    for (auto b : bullets_) {
        if (!b->alive) continue;
        IndexSet idx = map->GetMapChipIndexSetByPosition(b->pos);
        // guard against out-of-range indices
        uint32_t x = idx.xIndex;
        uint32_t y = idx.yIndex;
        if (x >= map->GetNumBlockHorizontal() || y >= map->GetNumBlockVertical()) continue;
        MapChipType t = map->GetMapChipTypeByIndex(x, y);
        if (t == MapChipType::kBlock || t == MapChipType::kIce) {
            b->alive = false;
        }
    }
}
