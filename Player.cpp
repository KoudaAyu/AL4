#include "Player.h"
#include<cassert>

using namespace KamataEngine;

void Player::Initialize(KamataEngine::Model* model, uint32_t texture, KamataEngine::Camera* camera) { 

	assert(model);
	model_ = model;
	assert(texture);
	textureHandle_ = texture;
	assert(camera);
	camera_ = camera;

	worldTransform_.Initialize();

}

void Player::Uppdate() { 
	worldTransform_.TransferMatrix();
}

void Player::Draw() {

	model_->Draw(worldTransform_, *camera_, textureHandle_);

}
