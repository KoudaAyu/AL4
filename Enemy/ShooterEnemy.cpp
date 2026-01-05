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

    if (ownsShooterModel_ && shooterModel_) {
        delete shooterModel_;
        shooterModel_ = nullptr;
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

    // create optional shooter model (e.g. "Shooter" or "Shooter.obj")
    shooterWorldTransform_.Initialize();
    shooterWorldTransform_.translation_ = pos;
    shooterWorldTransform_.rotation_ = worldTransform_.rotation_;
    shooterWorldTransform_.scale_ = {0.9f, 0.9f, 0.9f};

    shooterModel_ = Model::CreateFromOBJ("Shooter", true);
    if (shooterModel_) {
        ownsShooterModel_ = true;
    } else {
        // try fallback to "ShooterEnemy" or reuse body model
        shooterModel_ = Model::CreateFromOBJ("ShooterEnemy", true);
        if (shooterModel_) {
            ownsShooterModel_ = true;
        } else if (model_) {
            shooterModel_ = model_;
            ownsShooterModel_ = false;
        }
    }

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
    // NOTE: the model's forward axis is inverted in this project, so swap the angles
    if (faceRight_) {
        worldTransform_.rotation_.y = static_cast<float>(std::numbers::pi * 3.0 / 2.0);
    } else {
        worldTransform_.rotation_.y = static_cast<float>(std::numbers::pi / 2.0);
    }
    // keep shooter world rotation in sync
    shooterWorldTransform_.rotation_ = worldTransform_.rotation_;
}

void ShooterEnemy::Update() {
    if (!isAlive_) return;

    worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
    worldTransform_.TransferMatrix();

    // update shooter visual transform to match body
    shooterWorldTransform_.translation_ = worldTransform_.translation_;
    shooterWorldTransform_.rotation_ = worldTransform_.rotation_;
    shooterWorldTransform_.matWorld_ = MakeAffineMatrix(shooterWorldTransform_.scale_, shooterWorldTransform_.rotation_, shooterWorldTransform_.translation_);
    shooterWorldTransform_.TransferMatrix();

    // Only advance firing timer when shooting is allowed
    if (allowShooting_) {
        timer_ += 1.0f / 60.0f;
    }

    if (timer_ >= fireInterval_) {
        // Only spawn bullets when allowed
        if (allowShooting_) {
            timer_ = 0.0f;
            // determine facing from stored flag to ensure bullets follow configured facing
            bool facingRight = faceRight_;
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

    // draw optional shooter model (e.g. gun) in front of body
    if (shooterModel_ && camera_) shooterModel_->Draw(shooterWorldTransform_, *camera_);

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
