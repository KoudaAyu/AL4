#include "Enemy.h"
#include<cmath>
#include<numbers>
#include"MathUtl.h"
#include"Player.h"

using namespace KamataEngine;

Enemy::~Enemy() {
    if (ownsModel_ && model_) {
        delete model_;
        model_ = nullptr;
    }
}



void Enemy::Initialize(KamataEngine::Camera* camera, const KamataEngine::Vector3& pos) {
    model_ = Model::CreateFromOBJ("enemy", true);
    ownsModel_ = true;

    assert(model_);

    camera_ = camera;

    worldTransform_.Initialize();
    worldTransform_.translation_ = pos;
    worldTransform_.rotation_.y = -std::numbers::pi_v<float> / 2.0f;

    UpdateAABB();
}

void Enemy::Update() {

	if (!isAlive_)
	{
		return;
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
	(void)player;
	isAlive_ = false;
}
