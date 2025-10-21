#include "Player.h"

#include "KeyInput.h"
#include "MapChipField.h"
#include "MathUtl.h"
#include <algorithm>
#include <cassert>
#include <numbers>

using namespace KamataEngine;

void Player::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3 position) {

	assert(model);
	model_ = model;
	assert(camera);
	camera_ = camera;

	worldTransform_.Initialize();
	gridPos_.x = std::round(position.x / unitLength);
	gridPos_.y = std::round(position.y / unitLength);
	targetGridPos_ = gridPos_;

	// 方向も初期化（右向き）
	direction_ = {0.0f, 0.0f};      // 初期値は右向きでもOK
    nextDirection_ = {0.0f, 0.0f};  // 初期値は右向きでもOK
    isMoving_ = false;

	// worldTransform_もグリッドに合わせて再設定
	worldTransform_.translation_.x = (gridPos_.x + 0.5f) * unitLength;
	worldTransform_.translation_.y = (gridPos_.y + 0.5f) * unitLength;
	worldTransform_.translation_.z = position.z; // Zはそのまま

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


	// 死亡時は何もしない
	if (!isAlive_)
		return;

	// 入力取得
	KamataEngine::Vector2 lStick = KeyInput::GetInstance()->GetLStick();
	constexpr float stickThreshold = 0.5f;

	if (Input::GetInstance()->TriggerKey(DIK_SPACE) || KeyInput::GetInstance()->TriggerPadButton(XINPUT_GAMEPAD_A)) {
		if (bombActive_) {
			DetachBombParts();
		} else {
			RemoveLastPart();
		}
	}
	// 入力受付（移動中でなければ方向予約）

	if ((Input::GetInstance()->TriggerKey(DIK_LEFT) || lStick.x < -stickThreshold) && direction_.x != 1.0f) {
		nextDirection_ = {-1.0f, 0.0f};
	}
	if ((Input::GetInstance()->TriggerKey(DIK_RIGHT) || lStick.x > stickThreshold) && direction_.x != -1.0f) {
		nextDirection_ = {1.0f, 0.0f};
	}
	if ((Input::GetInstance()->TriggerKey(DIK_UP) || lStick.y > stickThreshold) && direction_.y != -1.0f) {
		nextDirection_ = {0.0f, 1.0f};
	}
	if ((Input::GetInstance()->TriggerKey(DIK_DOWN) || lStick.y < -stickThreshold) && direction_.y != 1.0f) {
		nextDirection_ = {0.0f, -1.0f};
	}

	// 移動開始判定
	if (!isMoving_) {
		KamataEngine::Vector2 nextGrid = {gridPos_.x + nextDirection_.x, gridPos_.y + nextDirection_.y};

		bool canMove = true;
		if (mapChipField_) {
			canMove = mapChipField_->IsMovable(static_cast<int>(nextGrid.x), static_cast<int>(nextGrid.y));
		}
		// 体との衝突判定もここで追加可能

		if (canMove) {
			direction_ = nextDirection_;
			targetGridPos_ = nextGrid;
			isMoving_ = true;
			moveTimer_ = 0.0f;
			startPos_ = {(gridPos_.x + 0.5f) * unitLength, (gridPos_.y + 0.5f) * unitLength, 0.0f};
			endPos_ = {(targetGridPos_.x + 0.5f) * unitLength, (targetGridPos_.y + 0.5f) * unitLength, 0.0f};

		}
	}

	// 補間移動
	if (isMoving_) {
		moveTimer_ += deltaTime_;
		float t = std::min(moveTimer_ / kMoveDuration, 1.0f);
		KamataEngine::Vector3 interpPos = startPos_ * (1.0f - t) + endPos_ * t;
		worldTransform_.translation_ = interpPos;

		// 頭の履歴を随時追加
		if (headHistory_.empty() || headHistory_.back() != worldTransform_.translation_) {
			headHistory_.push_back(worldTransform_.translation_);
		}

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
				if (historyIndex < headHistory_.size()) {
					bodyParts_[i] = headHistory_[historyIndex];
				}
			}
		}

		// 体パーツのTransform更新
		for (size_t i = 0; i < bodyParts_.size(); ++i) {
			bodyPartTransforms_[i].translation_ = bodyParts_[i];
			bodyPartTransforms_[i].scale_ = worldTransform_.scale_;
			bodyPartTransforms_[i].rotation_ = worldTransform_.rotation_;
			bodyPartTransforms_[i].matWorld_ = MakeAffineMatrix(bodyPartTransforms_[i].scale_, bodyPartTransforms_[i].rotation_, bodyPartTransforms_[i].translation_);
			bodyPartTransforms_[i].TransferMatrix();
		}

		if (t >= 1.0f) {
			// 移動完了
			gridPos_ = targetGridPos_;
			isMoving_ = false;
		}
	} else {
		// 静止時も体パーツのTransformを更新（消失防止）
		for (size_t i = 0; i < bodyParts_.size(); ++i) {
			bodyPartTransforms_[i].translation_ = bodyParts_[i];
			bodyPartTransforms_[i].scale_ = worldTransform_.scale_;
			bodyPartTransforms_[i].rotation_ = worldTransform_.rotation_;
			bodyPartTransforms_[i].matWorld_ = MakeAffineMatrix(bodyPartTransforms_[i].scale_, bodyPartTransforms_[i].rotation_, bodyPartTransforms_[i].translation_);
			bodyPartTransforms_[i].TransferMatrix();
		}
	}

	UpdateBomb();
	UpdateAABB();



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
		newPartPos = bodyParts_.back();
		Vector3 dir = {-direction_.x, -direction_.y, 0.0f};
		newPartPos.x += dir.x * unitLength;
		newPartPos.y += dir.y * unitLength;
	} else {
		newPartPos = worldTransform_.translation_;
	}

	bodyParts_.push_back(newPartPos);
	bodyPartTransforms_.emplace_back();
	bodyPartTransforms_.back().Initialize();
	bodyPartTransforms_.back().scale_ = worldTransform_.scale_;
	bodyPartTransforms_.back().rotation_ = worldTransform_.rotation_;
	bodyPartTransforms_.back().translation_ = bodyParts_.back();
	// 追加: matWorld_の更新と転送
	bodyPartTransforms_.back().matWorld_ = MakeAffineMatrix(bodyPartTransforms_.back().scale_, bodyPartTransforms_.back().rotation_, bodyPartTransforms_.back().translation_);
	bodyPartTransforms_.back().TransferMatrix();
}



void Player::RemoveLastPart() {
	if (bodyParts_.size() > 1) {
		KamataEngine::Vector3 removedPartPos = bodyParts_.back();

		// --- グリッドスナップ処理 ---
		int gridX = static_cast<int>(std::round(removedPartPos.x / unitLength - 0.5f));
		int gridY = static_cast<int>(std::round(removedPartPos.y / unitLength - 0.5f));
		KamataEngine::Vector3 snappedPos = {(gridX + 0.5f) * unitLength, (gridY + 0.5f) * unitLength, removedPartPos.z};
		// 壁Transformを追加
		wallTransforms_.emplace_back();
		wallTransforms_.back().Initialize();
		wallTransforms_.back().translation_ = snappedPos;
		wallTransforms_.back().scale_ = worldTransform_.scale_;
		wallTransforms_.back().rotation_ = worldTransform_.rotation_;
		wallTransforms_.back().matWorld_ = MakeAffineMatrix(wallTransforms_.back().scale_, wallTransforms_.back().rotation_, wallTransforms_.back().translation_);
		wallTransforms_.back().TransferMatrix();

		bodyParts_.pop_back();
		bodyPartTransforms_.pop_back();
	}
}


void Player::UpdateAABB() {
	constexpr float scale = 0.5f; // ← ここで縮小率を調整
	float half = (unitLength * scale) / 2.0f;
	const Vector3& pos = GetPosition();
	playerAABB.min = {pos.x - half, pos.y - half, pos.z - half};
	playerAABB.max = {pos.x + half, pos.y + half, pos.z + half};
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
	bombTimer_ += 1.0f / 60.0f;
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

	for (int i = 0; i < bombProgress_ && bodyParts_.size() > 1; ++i) {
		KamataEngine::Vector3 removedPartPos = bodyParts_.back();

		// --- グリッドインデックスを取得（-0.5f補正を追加） ---
		int gridX = static_cast<int>(std::round(removedPartPos.x / unitLength - 0.5f));
		int gridY = static_cast<int>(std::round(removedPartPos.y / unitLength - 0.5f));

		KamataEngine::Vector3 wallPos = removedPartPos;
		if (mapChipField_) {
			// マップチップの中心座標にスナップ
			wallPos = mapChipField_->GetMapChipPositionByIndex(gridX, gridY);
		} else {
			// フォールバック：グリッドの中心にスナップ
			wallPos = {(gridX + 0.5f) * unitLength, (gridY + 0.5f) * unitLength, removedPartPos.z};
		}

		wallTransforms_.emplace_back();
		wallTransforms_.back().Initialize();
		wallTransforms_.back().translation_ = wallPos;
		wallTransforms_.back().scale_ = worldTransform_.scale_;
		wallTransforms_.back().rotation_ = worldTransform_.rotation_;
		wallTransforms_.back().matWorld_ = MakeAffineMatrix(wallTransforms_.back().scale_, wallTransforms_.back().rotation_, wallTransforms_.back().translation_);
		wallTransforms_.back().TransferMatrix();

		bodyParts_.pop_back();
		bodyPartTransforms_.pop_back();
	}

	bombActive_ = false;
	bombProgress_ = 0;
}

bool Player::IsOccupyingGrid(int gridX, int gridY) const {
	auto posToGrid = [&](const KamataEngine::Vector3& p) -> std::pair<int, int> {
		int gx = static_cast<int>(std::round(p.x / unitLength - 0.5f));
		int gy = static_cast<int>(std::round(p.y / unitLength - 0.5f));
		return {gx, gy};
	};

	// 頭・体（bodyParts_には頭も含まれる想定）
	for (const auto& p : bodyParts_) {
		auto [bx, by] = posToGrid(p);
		if (bx == gridX && by == gridY) {
			return true;
		}
	}

	// 設置済みの壁
	for (const auto& wt : wallTransforms_) {
		auto [wx, wy] = posToGrid(wt.translation_);
		if (wx == gridX && wy == gridY) {
			return true;
		}
	}

	return false;
}
