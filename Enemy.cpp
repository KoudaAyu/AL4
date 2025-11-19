#include "Enemy.h"
#include<cmath>
#include<numbers>
#include"MathUtl.h"

using namespace KamataEngine;

Enemy::~Enemy() {}

void Enemy::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& pos) {

	assert(model);
	// 引数として受け取ったデータをメンバ関数に記録
	model_ = model;
	/*textureHandle_ = textureHandle;*/
	camera_ = camera;
	// ワールド変換の初期化
	worldTransform_.Initialize();
	worldTransform_.translation_ = pos;
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;

}

void Enemy::Update() {

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();
}

void Enemy::Draw() {

	model_->Draw(worldTransform_, *camera_); }
