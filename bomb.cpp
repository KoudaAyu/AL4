#include "bomb.h"
#include "MathUtl.h"

using namespace KamataEngine;

void Bomb::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position) {
	model_ = model;
	camera_ = camera;
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;

	textureHandle_ = TextureManager::Load("Blue.png");
}


void Bomb::Update() {
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();
}

void Bomb::Draw() { 
	model_->Draw(worldTransform_, *camera_, textureHandle_);
}
