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
    // 左向きにする
    worldTransform_.rotation_.y = -std::numbers::pi_v<float> / 2.0f;

    UpdateAABB();
}

void FrontShieldEnemy::OnCollision(Player* player) {
    if (!player) return;

    if (player->GetLRDirection() == Player::LRDirection::kLeft && lrDirection_ == FrontShieldEnemy::LRDirection::kRight) {
        // プレイヤーが左向きで敵も左向きの場合、衝突を無効化
        return;
    }

    if (player->GetLRDirection() == Player::LRDirection::kRight && lrDirection_ == FrontShieldEnemy::LRDirection::kLeft) {
        // プレイヤーが右向きで敵も右向きの場合、衝突を無効化
        return;
	}

    Enemy::OnCollision(player);
}
