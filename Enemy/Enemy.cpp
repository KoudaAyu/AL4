#include "Enemy.h"
#include<cmath>
#include<numbers>
#include"../MathUtl.h"
#include"../Player.h"
#include"../MapChipField.h"

using namespace KamataEngine;

Enemy::~Enemy() {
    if (ownsModel_ && model_) {
        delete model_;
        model_ = nullptr;
    }
}

void Enemy::Initialize(KamataEngine::Camera* camera, const KamataEngine::Vector3& pos) {
    // default: face right (existing behavior)
    Initialize(camera, pos, false);
}

// Initialize with facing option (faceLeft = true makes enemy face left on spawn)
void Enemy::Initialize(KamataEngine::Camera* camera, const KamataEngine::Vector3& pos, bool faceLeft) {
    model_ = Model::CreateFromOBJ("enemy", true);
    ownsModel_ = true;

    assert(model_);

    camera_ = camera;

    worldTransform_.Initialize();
    worldTransform_.translation_ = pos;
    // existing default used -pi/2 means facing right in this project convention
    if (faceLeft) {
        // rotate to face left: flip yaw by adding pi (180deg)
        worldTransform_.rotation_.y = -std::numbers::pi_v<float> / 2.0f + std::numbers::pi_v<float>;
        facingRight_ = false;
    } else {
        worldTransform_.rotation_.y = -std::numbers::pi_v<float> / 2.0f;
        facingRight_ = true;
    }

    speed_ = 0.12f;
    velocityX_ = (facingRight_ ? speed_ : -speed_);

    UpdateAABB();
}

void Enemy::SetMapChipField(MapChipField* map) {
    mapChipField_ = map;
}

void Enemy::SetFacingRight(bool facing) {
    facingRight_ = facing;
    velocityX_ = (facingRight_ ? std::fabs(velocityX_) : -std::fabs(velocityX_));
    // adjust rotation to visually face direction
    if (facingRight_) {
        worldTransform_.rotation_.y = -std::numbers::pi_v<float> / 2.0f;
    } else {
        worldTransform_.rotation_.y = -std::numbers::pi_v<float> / 2.0f + std::numbers::pi_v<float>;
    }
}

void Enemy::Update() {

	if (!isAlive_)
	{
		return;
	}

	// Apply horizontal movement
	// Move by velocityX_ each frame
	worldTransform_.translation_.x += velocityX_;

	// Simple collision with map blocks: if the enemy intersects a block after moving, reverse direction and step back
	if (mapChipField_) {
		// compute current AABB after move
		UpdateAABB();
		IndexSet idxMin = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_);
		// Check the block under the enemy center and adjacent blocks in facing direction
		int checkX = static_cast<int>(idxMin.xIndex);
		int checkY = static_cast<int>(idxMin.yIndex);

		// If there's a block at the index we occupy, then we collided horizontally with block; reverse
		MapChipType t = mapChipField_->GetMapChipTypeByIndex(checkX, checkY);
		if (t == MapChipType::kBlock || t == MapChipType::kIce) {
			// Move back and reverse
			worldTransform_.translation_.x -= velocityX_;
			SetFacingRight(!facingRight_);
		} else {
			// additionally check one tile ahead in facing direction at foot level to detect cliffs
			int aheadX = checkX + (facingRight_ ? 1 : -1);
			int belowAheadY = checkY + 1; // below in grid coordinates (since map's yIndex increases downward)
			// If tile ahead is blank (fall) and tile below ahead is also blank, reverse to avoid walking off cliffs
			MapChipType aheadType = mapChipField_->GetMapChipTypeByIndex(aheadX, checkY);
			MapChipType belowAheadType = mapChipField_->GetMapChipTypeByIndex(aheadX, belowAheadY);
			if ((aheadType == MapChipType::kBlank) && (belowAheadType == MapChipType::kBlank)) {
				// reverse instead of falling
				worldTransform_.translation_.x -= velocityX_;
				SetFacingRight(!facingRight_);
			}
		}
	}

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();
	// 毎フレームAABB更新
	UpdateAABB();
}

void Enemy::Draw() {
	if (!isAlive_)
	{
		return;
	}
	model_->Draw(worldTransform_, *camera_);
}

void Enemy::UpdateAABB() {
	// プレイヤーと同等サイズの簡易AABB（必要なら調整）
	static constexpr float kWidth = 0.8f * 2.0f;
	static constexpr float kHeight = 0.8f * 2.0f;
	static constexpr float kDepth = 0.8f * 2.0f;

	Vector3 center = worldTransform_.translation_;
	Vector3 half = {kWidth * 0.5f, kHeight * 0.5f, kDepth * 0.5f};

	// 回転は無視して軸整列AABBを更新（必要ならメッシュ頂点から算出実装へ拡張）
	aabb_.min = {center.x - half.x, center.y - half.y, center.z - half.z};
	aabb_.max = {center.x + half.x, center.y + half.y, center.z + half.z};
}

void Enemy::OnCollision(Player* player) {

	if (player && player->IsAttacking()) {
		isAlive_ = false;
	}
}
