#include "Player.h"

#include <algorithm>
#include<cassert>
#include <numbers>

#include "MathUtl.h"

using namespace KamataEngine;

void Player::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3 position) { 

	assert(model);
	model_ = model;
	assert(camera);
	camera_ = camera;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	//初期の向き
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;

}

void Player::Update() { 

	if (Input::GetInstance()->PushKey(DIK_RIGHT)||Input::GetInstance()->PushKey(DIK_LEFT))
	{
		Vector3 acceleration = {};
		if (Input::GetInstance()->PushKey(DIK_RIGHT))
		{
			if (velocity_.x < 0.0f)
			{
				velocity_.x *= (1.0f - kAttenuation);
			}
			acceleration.x += kAcceleration;
		} else if (Input::GetInstance()->PushKey(DIK_LEFT)) {
			acceleration.x -= kAcceleration;
			if (velocity_.x > 0.0f) {
				velocity_.x *= (1.0f - kAttenuation);
			}
		}
		velocity_ += acceleration;
		// 速度制限
		velocity_.x = std::clamp(velocity_.x, -kLimitSpeed, kLimitSpeed);
	}
	else
	{
		velocity_.x *= (1.0f - kAttenuation);

	}

	worldTransform_.translation_ += velocity_;

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();
}

void Player::Draw() {

	model_->Draw(worldTransform_, *camera_, textureHandle_);

}
