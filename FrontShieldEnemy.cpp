#include "FrontShieldEnemy.h"
#include "Player.h"
#include "MathUtl.h"

using namespace KamataEngine;

FrontShieldEnemy::~FrontShieldEnemy() {
    if (ownsShieldModel_ && shieldModel_) {
        delete shieldModel_;
        shieldModel_ = nullptr;
    }
}

void FrontShieldEnemy::Initialize(KamataEngine::Camera* camera, const KamataEngine::Vector3& pos) {
    // Create OBJ model similarly to Player
   
  
    // Create body OBJ model similarly to Player/Enemy
    model_ = Model::CreateFromOBJ("FrontShieldEnemy", true);
    ownsModel_ = true;

    assert(model_);

    // create separate shield model (reuse same mesh name if present, otherwise fallback to same model)
    shieldModel_ = Model::CreateFromOBJ("Enemy", true);
    if (!shieldModel_) {
        // fallback to body model so at least something is drawn
        shieldModel_ = model_;
        ownsShieldModel_ = false;
    } else {
        ownsShieldModel_ = true;
    }

    camera_ = camera;

    // ワールド変換の初期化
    worldTransform_.Initialize();
    worldTransform_.translation_ = pos;
    // 左向きにする
    worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
    // Use same facing convention as other enemies: right -> pi/2, left -> 3*pi/2
    worldTransform_.rotation_.y = static_cast<float>(std::numbers::pi / 2.0);

    // set logical facing to match visual rotation
    lrDirection_ = (std::sin(worldTransform_.rotation_.y) > 0.0f) ? LRDirection::kRight : LRDirection::kLeft;

    // initialize shield transform relative to body
    shieldWorldTransform_.Initialize();
    shieldWorldTransform_.translation_ = pos; // same base position
    // offset the shield slightly in front of the enemy along x depending on facing
    shieldWorldTransform_.scale_ = {1.0f, 1.0f, 1.0f};
    shieldWorldTransform_.rotation_ = worldTransform_.rotation_;

    // small offset along local +x to place shield in front visually
    float forwardOffset = 0.5f; // bring shield closer for 2D-style view
    // small Z bias to avoid z-fighting (bring shield slightly toward camera)
    float shieldZBias = -0.01f; // minimal bias for 2D layering
    // compute offset in world space using facing
    bool facingRight = (std::sin(worldTransform_.rotation_.y) > 0.0f);
    shieldWorldTransform_.translation_.x += facingRight ? forwardOffset : -forwardOffset;
    shieldWorldTransform_.translation_.z += shieldZBias;

    UpdateAABB();
}

void FrontShieldEnemy::OnCollision(Player* player) {
    if (!player) return;

    // If the player is attacking and facing the same direction as the enemy (i.e. attacking from front),
    // shield should block the attack and prevent enemy from being damaged.
    if (player->IsAttacking() &&
        ((player->GetLRDirection() == Player::LRDirection::kLeft && lrDirection_ == FrontShieldEnemy::LRDirection::kLeft) ||
         (player->GetLRDirection() == Player::LRDirection::kRight && lrDirection_ == FrontShieldEnemy::LRDirection::kRight))) {
        Audio::GetInstance()->PlayWave(soundDataHandle, false);
        return; // block enemy getting hit
    }

    // Otherwise, process collision normally (this will damage player if appropriate)
    Enemy::OnCollision(player);
}

void FrontShieldEnemy::Update() {
    if (!isAlive_) return;

    // update body transform matrix
    worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
    worldTransform_.TransferMatrix();

    // ensure shield follows body each frame
    shieldWorldTransform_.rotation_ = worldTransform_.rotation_;
    shieldWorldTransform_.translation_ = worldTransform_.translation_;
    bool facingRight = (std::sin(worldTransform_.rotation_.y) > 0.0f);
    float forwardOffset = 0.5f; // match Initialize (closer)
    float shieldZBias = -0.01f; // match Initialize
    shieldWorldTransform_.translation_.x += facingRight ? forwardOffset : -forwardOffset;
    shieldWorldTransform_.translation_.z = worldTransform_.translation_.z + shieldZBias;
    shieldWorldTransform_.matWorld_ = MakeAffineMatrix(shieldWorldTransform_.scale_, shieldWorldTransform_.rotation_, shieldWorldTransform_.translation_);
    shieldWorldTransform_.TransferMatrix();

    // 更新AABB
    UpdateAABB();
}

void FrontShieldEnemy::Draw() {
    if (!isAlive_) return;
    if (model_ && camera_) model_->Draw(worldTransform_, *camera_);
    if (shieldModel_ && camera_) shieldModel_->Draw(shieldWorldTransform_, *camera_);
}
