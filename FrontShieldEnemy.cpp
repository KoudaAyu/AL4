#include "FrontShieldEnemy.h"
#include "Player.h"
#include "MathUtl.h"

using namespace KamataEngine;

void FrontShieldEnemy::Initialize(KamataEngine::Camera* camera, const KamataEngine::Vector3& pos) {
    // Create OBJ model similarly to Player
    model_ = Model::CreateFromOBJ("enemy", true);
    ownsModel_ = true;

    assert(model_);

    camera_ = camera;

    // ワールド変換の初期化
    worldTransform_.Initialize();
    worldTransform_.translation_ = pos;
    worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;

    UpdateAABB();
}

void FrontShieldEnemy::OnCollision(Player* player) {
    if (!player) return;

    Vector3 enemyPos = worldTransform_.translation_;
    Vector3 playerPos = player->GetPosition();
    Vector3 toPlayer = playerPos - enemyPos;
    toPlayer.z = 0.0f;

    float lenSq = toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y;
    if (lenSq == 0.0f) {
        Enemy::OnCollision(player);
        return;
    }
    float invLen = 1.0f / std::sqrt(lenSq);
    toPlayer.x *= invLen;
    toPlayer.y *= invLen;

   
    float ry = worldTransform_.rotation_.y;
    float adj = ry - std::numbers::pi_v<float> / 2.0f;
    Vector3 forward = { std::cos(adj), std::sin(adj), 0.0f };

    float dot = forward.x * toPlayer.x + forward.y * toPlayer.y;

    if (dot > frontDotThreshold_) {
        return;
    }

    Enemy::OnCollision(player);
}
