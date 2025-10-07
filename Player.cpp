#include "Player.h"

#include <algorithm>
#include <cassert>
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

	bodyParts_.clear();
	bodyParts_.push_back(worldTransform_.translation_);

	headHistory_.clear();
	for (size_t i = 0; i < kFollowDelay * bodyParts_.size(); ++i) {
		headHistory_.push_back(worldTransform_.translation_);
	}

	bodyPartTransforms_.emplace_back();
	bodyPartTransforms_.back().Initialize();
	bodyPartTransforms_.back().scale_ = worldTransform_.scale_;
	bodyPartTransforms_.back().rotation_ = worldTransform_.rotation_;
	bodyPartTransforms_.back().translation_ = bodyParts_.back();

	// 初期の向き
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
}

void Player::Update() {

	if (Input::GetInstance()->PushKey(DIK_RIGHT) || Input::GetInstance()->PushKey(DIK_LEFT)) {
		Vector3 acceleration = {};
		if (Input::GetInstance()->PushKey(DIK_RIGHT)) {
			if (velocity_.x < 0.0f) {
				velocity_.x *= (1.0f - kAttenuation);
			}
			acceleration.x += kAcceleration;
			if (lrDirection_ != LRDirection::Right) {
				lrDirection_ = LRDirection::Right;
				lrKnown_ = true;
				udKnown_ = false;
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
				lrKnown_ = true;
				udKnown_ = false;
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

	if (Input::GetInstance()->PushKey(DIK_UP) || Input::GetInstance()->PushKey(DIK_DOWN)) {
		Vector3 acceleration = {};
		if (Input::GetInstance()->PushKey(DIK_UP)) {
			if (velocity_.y < 0.0f) {
				velocity_.y *= (1.0f - kAttenuation);
			}
			acceleration.y += kAcceleration;
			if (udDirection_ != UDDirection::Up) {
				udDirection_ = UDDirection::Up;
				udKnown_ = true;
				lrKnown_ = false;
			}
		} else if (Input::GetInstance()->PushKey(DIK_DOWN)) {
			if (velocity_.y > 0.0f) {
				velocity_.y *= (1.0f - kAttenuation);
			}
			acceleration.y -= kAcceleration;
			if (udDirection_ != UDDirection::Down) {
				udDirection_ = UDDirection::Down;
				udKnown_ = true;
				lrKnown_ = false;
			}
		}
		velocity_ += acceleration;
		// 速度制限
		velocity_.y = std::clamp(velocity_.y, -kLimitSpeed, kLimitSpeed);
	} else {
		velocity_.y *= (1.0f - kAttenuation);
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

	// 頭の現在位置を履歴に追加
	headHistory_.push_back(worldTransform_.translation_);

	// 履歴が長すぎる場合は古いものを削除
	size_t requiredHistory = kFollowDelay * bodyParts_.size();
	while (headHistory_.size() > requiredHistory) {
		headHistory_.pop_front();
	}

	// 各パーツを遅延追従させる
	for (size_t i = 0; i < bodyParts_.size(); ++i) {
		if (i == 0) {
			bodyParts_[0] = worldTransform_.translation_; // 頭は現在位置
		} else {
			size_t historyIndex = headHistory_.size() - 1 - i * kFollowDelay;
			bodyParts_[i] = headHistory_[historyIndex]; // 体は過去の座標
		}
	}

	for (size_t i = 0; i < bodyParts_.size(); ++i) {
		bodyPartTransforms_[i].translation_ = bodyParts_[i];
		bodyPartTransforms_[i].scale_ = worldTransform_.scale_;
		bodyPartTransforms_[i].rotation_ = worldTransform_.rotation_;
		bodyPartTransforms_[i].matWorld_ = MakeAffineMatrix(bodyPartTransforms_[i].scale_, bodyPartTransforms_[i].rotation_, bodyPartTransforms_[i].translation_);
		bodyPartTransforms_[i].TransferMatrix();
	}

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();
}

void Player::Draw() {
	// 頭（プレイヤー本体）は今まで通り
	model_->Draw(worldTransform_, *camera_, textureHandle_);

	// 体（1以降）を描画
	for (size_t i = 0; i < bodyPartTransforms_.size(); ++i) {
		model_->Draw(bodyPartTransforms_[i], *camera_, textureHandle_);
	}
}

// Player.cpp
void Player::Grow() {
	KamataEngine::Vector3 newPartPos;
	if (!bodyParts_.empty()) {
		// 末尾のパーツの座標を取得
		newPartPos = bodyParts_.back();
	} else {
		// まだパーツが無い場合はヘッドの座標
		newPartPos = worldTransform_.translation_;
	}

	// 進行方向の逆側にunitLength分ずらす
	// 例: 右向きなら左へ、上向きなら下へ
	if (lrDirection_ == LRDirection::Right) {
		newPartPos.x -= unitLength;
	} else if (lrDirection_ == LRDirection::Left) {
		newPartPos.x += unitLength;
	}
	if (udDirection_ == UDDirection::Up) {
		newPartPos.y -= unitLength;
	} else if (udDirection_ == UDDirection::Down) {
		newPartPos.y += unitLength;
	}

	bodyParts_.push_back(newPartPos);
	for (size_t i = 0; i < kFollowDelay; ++i) {
		headHistory_.push_front(bodyParts_.back());
	}

	// 体パーツ用のWorldTransformも追加・初期化
	bodyPartTransforms_.emplace_back();
	bodyPartTransforms_.back().Initialize();
	bodyPartTransforms_.back().scale_ = worldTransform_.scale_;
	bodyPartTransforms_.back().rotation_ = worldTransform_.rotation_;
	bodyPartTransforms_.back().translation_ = bodyParts_.back();
}
