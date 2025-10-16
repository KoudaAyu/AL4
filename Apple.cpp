#include "Apple.h"
#include "MathUtl.h"

using namespace KamataEngine;

void Apple::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position) {
	model_ = model;
	camera_ = camera;
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;

	textureHandle_ = TextureManager::Load("Red.png");
}

void Apple::Update() {
	// 必要ならアニメーションやロジック
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();
}

void Apple::Draw() {
	if (isActive_ && model_) {
		model_->Draw(worldTransform_, *camera_,textureHandle_);
	}
}

void Apple::Respawn(const KamataEngine::Vector3 RespawnPosition) {

	worldTransform_.translation_ = RespawnPosition;
	isActive_ = true; 
}
