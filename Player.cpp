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

	
	redColor.Initialize();

	
	normalColor.Initialize();
}

void Player::Update() {

	// スペースキーで最後のパーツを削除
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		if (bombActive_) {
			DetachBombParts();
		} else {
			RemoveLastPart();
		}
	}
	UpdateBomb();
	playerAABB.min = worldTransform_.translation_ - KamataEngine::Vector3{0.5f, 0.5f, 0.5f};
	playerAABB.max = worldTransform_.translation_ + KamataEngine::Vector3{0.5f, 0.5f, 0.5f};

	for (const auto& wallTransform : wallTransforms_) {
		AABB wallAABB;
		wallAABB.min = wallTransform.translation_ - KamataEngine::Vector3{0.5f, 0.5f, 0.5f};
		wallAABB.max = wallTransform.translation_ + KamataEngine::Vector3{0.5f, 0.5f, 0.5f};

		if (IsCollisionAABBAABB(playerAABB, wallAABB)) {
			/*__debugbreak()*/; // デバッグ時のみ
		}
	}


	// 左右キー入力処理
	if (Input::GetInstance()->TriggerKey(DIK_LEFT) && lrDirection_ != LRDirection::Right) {
		lrDirection_ = LRDirection::Left;
		lrKnown_ = true;
		udKnown_ = false;
		udDirection_ = UDDirection::Unknown;
		turnFirstRotationY_ = worldTransform_.rotation_.y;
		turnTimer_ = kTimeTurn;
	}

	if (Input::GetInstance()->TriggerKey(DIK_RIGHT) && lrDirection_ != LRDirection::Left) {
		lrDirection_ = LRDirection::Right;
		lrKnown_ = true;
		udKnown_ = false;
		udDirection_ = UDDirection::Unknown;
		turnFirstRotationY_ = worldTransform_.rotation_.y;
		turnTimer_ = kTimeTurn;
	}

	// 上下キー入力処理
	if (Input::GetInstance()->TriggerKey(DIK_UP) && udDirection_ != UDDirection::Down) {
		udDirection_ = UDDirection::Up;
		udKnown_ = true;
		lrKnown_ = false;
		lrDirection_ = LRDirection::Unknown;

	
	}

	if (Input::GetInstance()->TriggerKey(DIK_DOWN) && udDirection_ != UDDirection::Up) {
		udDirection_ = UDDirection::Down;
		udKnown_ = true;
		lrKnown_ = false;

		lrDirection_ = LRDirection::Unknown;
	}

	// 進行方向に自動で移動
	velocity_ = {0.0f, 0.0f, 0.0f};
	float speed = kLimitSpeed;

	if (lrKnown_) {
		velocity_.x = (lrDirection_ == LRDirection::Right) ? speed : -speed;
		velocity_.y = 0.0f;
	} else if (udKnown_) {
		velocity_.y = (udDirection_ == UDDirection::Up) ? speed : -speed;
		velocity_.x = 0.0f;
	}

	worldTransform_.translation_ += velocity_;

	if (lrKnown_) { // ←★ 左右方向のときだけ回転処理を行う
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
			float destinationRotationYTable[] = {
			    std::numbers::pi_v<float> / 2.0f,       // Right
			    std::numbers::pi_v<float> * 3.0f / 2.0f // Left
			};
			worldTransform_.rotation_.y = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];
		}
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
			bodyParts_[0] = worldTransform_.translation_;
		} else {
			size_t historyIndex = headHistory_.size() - 1 - i * kFollowDelay;
			bodyParts_[i] = headHistory_[historyIndex];
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
	if (isAlive_) {
		redColor.SetColor({1.0f, 0.0f, 0.0f, 1.0f});
		normalColor.SetColor({1.0f, 1.0f, 1.0f, 1.0f});

		// 頭（プレイヤー本体）は常に通常色
		model_->Draw(worldTransform_, *camera_, textureHandle_, &normalColor);

		// 体パーツ
		for (size_t i = 0; i < bodyPartTransforms_.size(); ++i) {
			const ObjectColor* colorToUse = &normalColor;
			if (bombActive_ && i != 0) { // 頭は常に白
				if (i >= bodyPartTransforms_.size() - bombProgress_) {
					colorToUse = &redColor;
				}
			}
			model_->Draw(bodyPartTransforms_[i], *camera_, textureHandle_, colorToUse);
		}

		// 壁も同じ色で描画（必要なら）
		for (const auto& wallTransform : wallTransforms_) {
			model_->Draw(wallTransform, *camera_, textureHandle_, &normalColor);
		}
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

void Player::RemoveLastPart() {
	if (bodyParts_.size() > 1) {
		// 削除する直前のパーツの座標を保存
		KamataEngine::Vector3 removedPartPos = bodyParts_.back();

		// 壁のTransformを追加し、座標を設定
		wallTransforms_.emplace_back();
		wallTransforms_.back().Initialize();
		wallTransforms_.back().translation_ = removedPartPos;
		wallTransforms_.back().scale_ = worldTransform_.scale_;       // 必要ならスケールもコピー
		wallTransforms_.back().rotation_ = worldTransform_.rotation_; // 必要なら回転もコピー
		wallTransforms_.back().matWorld_ = MakeAffineMatrix(wallTransforms_.back().scale_, wallTransforms_.back().rotation_, wallTransforms_.back().translation_);
		wallTransforms_.back().TransferMatrix();

		bodyParts_.pop_back();
		bodyPartTransforms_.pop_back();
	}
}

void Player::EatBomb() {
	if (bombActive_)
		return; // すでに爆弾進行中なら何もしない

	if (bodyParts_.size() <= 1) {
		// 頭だけの場合は即死
		isAlive_ = false;
		return;
	}

	// 体がある場合は爆弾進行開始
	bombActive_ = true;
	bombProgress_ = 1; // ←ここを1にする
	bombStartIndex_ = static_cast<int>(bodyParts_.size()) - 1;
	bombTimer_ = 0.0f;
}



void Player::UpdateBomb() {
	if (!bombActive_)
		return;
	bombTimer_ += deltaTime_;
	if (bombTimer_ >= kBombStepTime) {
		bombTimer_ = 0.0f;
		bombProgress_++;
		if (bombProgress_ >= bodyParts_.size()) {
			// ゲームオーバー
			isAlive_ = false;
		}
	}
}

void Player::DetachBombParts() {
	if (!bombActive_ || bombProgress_ == 0)
		return;

	// bombProgress_分だけ体を切り離し、壁に追加
	for (int i = 0; i < bombProgress_ && bodyParts_.size() > 1; ++i) {
		// 体の最後（尻尾側）を壁に
		KamataEngine::Vector3 removedPartPos = bodyParts_.back();

		wallTransforms_.emplace_back();
		wallTransforms_.back().Initialize();
		wallTransforms_.back().translation_ = removedPartPos;
		wallTransforms_.back().scale_ = worldTransform_.scale_;
		wallTransforms_.back().rotation_ = worldTransform_.rotation_;
		wallTransforms_.back().matWorld_ = MakeAffineMatrix(wallTransforms_.back().scale_, wallTransforms_.back().rotation_, wallTransforms_.back().translation_);
		wallTransforms_.back().TransferMatrix();

		bodyParts_.pop_back();
		bodyPartTransforms_.pop_back();
	}

	// 爆弾状態リセット
	bombActive_ = false;
	bombProgress_ = 0;
}
