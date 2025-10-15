#include "Player.h"

#include <algorithm>
#include <cassert>
#include <numbers>
#include"KeyInput.h"
#include "MathUtl.h"

using namespace KamataEngine;

void Player::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3 position) {

	assert(model);
	model_ = model;
	assert(camera);
	camera_ = camera;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	// 初期の向き
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
}

void Player::Update() {

	// XBoxコントローラーの左スティック入力取得
	Vector2 lStick = KeyInput::GetInstance()->GetLStick();

	// スティックのデッドゾーン処理（必要に応じて）
	if (fabs(lStick.x) > 0.1f) {
		velocity_.x += lStick.x * kAcceleration;
		velocity_.x = std::clamp(velocity_.x, -kLimitSpeed, kLimitSpeed);
	}

	if (Input::GetInstance()->PushKey(DIK_RIGHT) || Input::GetInstance()->PushKey(DIK_LEFT)) {
		Vector3 acceleration = {};
		if (Input::GetInstance()->PushKey(DIK_RIGHT)) {
			if (velocity_.x < 0.0f) {
				velocity_.x *= (1.0f - kAttenuation);
			}
			acceleration.x += kAcceleration;
			if (lrDirection_ != LRDirection::Right) {
				lrDirection_ = LRDirection::Right;
				turnFirstRotationY_ = worldTransform_.rotation_.y;
				turnTimer_ = kTimeTurn;
			}

		} else if (Input::GetInstance()->PushKey(DIK_LEFT)) {
			if (velocity_.x > 0.0f) {
				velocity_.x *= (1.0f - kAttenuation);
			}
			acceleration.x -= kAcceleration;
			if (lrDirection_ != LRDirection::Left) {
				lrDirection_ = LRDirection::Left;
				turnFirstRotationY_ = worldTransform_.rotation_.y;
				turnTimer_ = kTimeTurn;
			}
		}
		velocity_ += acceleration;
		// 速度制限
		velocity_.x = std::clamp(velocity_.x, -kLimitSpeed, kLimitSpeed);
	} else {
		velocity_.x *= (1.0f - kAttenuation);
	}

	worldTransform_.translation_ += velocity_;

	if (turnTimer_ > 0.0f) {
		turnTimer_ -= 1.0f / 60.0f;
		turnTimer_ = std::max(turnTimer_, 0.0f);

		float t = 1.0f - (turnTimer_ / kTimeTurn);
		float easeT = 1.0f - powf(1.0f - t, 3.0f);

		float destinationRotationYTable[] = {
		    std::numbers::pi_v<float> / 2.0f,       // Right
		    std::numbers::pi_v<float> * 3.0f / 2.0f // Left
		};
		float destination = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];
		worldTransform_.rotation_.y = turnFirstRotationY_ + (destination - turnFirstRotationY_) * easeT;
	} else {
		float destinationRotationYTable[] = {std::numbers::pi_v<float> / 2.0f, std::numbers::pi_v<float> * 3.0f / 2.0f};
		worldTransform_.rotation_.y = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];
	}

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();
}

void Player::Draw() { model_->Draw(worldTransform_, *camera_, textureHandle_); }
