#include "Player.h"

#include "KeyInput.h"
#include "MapChipField.h"
#include "MathUtl.h"
#include <algorithm>
#include <cassert>
#include <numbers>

#include <Xinput.h>
#pragma comment(lib, "xinput.lib")

using namespace KamataEngine;



namespace {
// Simple rumble controller for player 1 (index 0)
struct RumbleState {
	bool active = false;
	ULONGLONG endTick = 0; // GetTickCount64 based end time
};

RumbleState g_rumble;

WORD ToMotor(float v) {
	v = std::clamp(v, 0.0f, 1.0f);
	return static_cast<WORD>(v * 65535.0f);
}

void StartRumble(float left, float right, int durationMs, DWORD userIndex = 0) {
	XINPUT_VIBRATION vib{};
	vib.wLeftMotorSpeed = ToMotor(left);
	vib.wRightMotorSpeed = ToMotor(right);
	XInputSetState(userIndex, &vib);
	g_rumble.active = true;
	g_rumble.endTick = GetTickCount64() + static_cast<ULONGLONG>(std::max(durationMs, 0));
}

void StopRumble(DWORD userIndex = 0) {
	XINPUT_VIBRATION vib{}; // zeros stop the motors
	XInputSetState(userIndex, &vib);
	g_rumble.active = false;
}

void UpdateRumble(DWORD userIndex = 0) {
	if (g_rumble.active) {
		ULONGLONG now = GetTickCount64();
		if (now >= g_rumble.endTick) {
			StopRumble(userIndex);
		}
	}
}
} // namespace

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

	Move();

	// 衝突判定を初期化
	CollisionMapInfo collisionpMaInfo;
	// 移動量に速度の値をコピーする
	collisionpMaInfo.movement = velocity_;

	// マップ衝突判定チェック
	CollisionMap(collisionpMaInfo);

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

void Player::Move() {
	// XBoxコントローラーの左スティック入力取得
	Vector2 lStick = KeyInput::GetInstance()->GetLStick();

	// スティックのデッドゾーン処理（必要に応じて）
	if (fabs(lStick.x) > 0.1f) {
		velocity_.x += lStick.x * kAcceleration;
		velocity_.x = std::clamp(velocity_.x, -kLimitSpeed, kLimitSpeed);
	}

	if (KeyInput::GetInstance()->TriggerPadButton(XINPUT_GAMEPAD_A) && jumpCount_ < kMaxJumpCount) {
		isJump_ = true;
		jumpVelocity_ = kJumpVelocity;
		jumpCount_++;
	}

	// キー入力による移動
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

	Jump();
}

void Player::Jump() {

	if (Input::GetInstance()->TriggerKey(DIK_SPACE) && jumpCount_ < kMaxJumpCount) {
		isJump_ = true;
		jumpVelocity_ = kJumpVelocity;
		jumpCount_++;
	}

	// ジャンプ中の重力処理
	if (isJump_) {
		jumpVelocity_ -= kGravity;
		worldTransform_.translation_.y += jumpVelocity_;

		// 着地判定（y=0が地面）
		if (worldTransform_.translation_.y <= 0.0f) {
			worldTransform_.translation_.y = 0.0f;
			isJump_ = false;
			jumpVelocity_ = 0.0f;
			jumpCount_ = 0;
		}
	}
}

void Player::CollisionMap(CollisionMapInfo& collisionMapInfo) {

	CollisionMapUp(collisionMapInfo);
	CollisionMapDown(collisionMapInfo);
	CollisionMapLeft(collisionMapInfo);
	CollisionMapRight(collisionMapInfo);
}

void Player::CollisionMapUp(CollisionMapInfo& collisionMapInfo) {

	// 上方向へ移動していないなら判定不要
	if (jumpVelocity_ <= 0.0f) {
		return;
	}

	// プレイヤーの4つのコーナーの位置を計算（現在位置で判定）
	std::array<Vector3, kNumCorners> cornerPositions{};
	for (uint32_t i = 0; i < cornerPositions.size(); ++i) {
		cornerPositions[i] = CornerPisition(worldTransform_.translation_, static_cast<Corner>(i));
	}

	// 真上の当たり判定を行う（左右の上コーナー）
	bool leftHit = false;
	bool rightHit = false;
	MapChipField::IndexSet leftIndex = mapChipField_->GetMapChipIndexByPosition(cornerPositions[kLeftTop]);
	MapChipType leftType = mapChipField_->GetMapChipTypeByIndex(leftIndex.xIndex, leftIndex.yIndex);
	if (leftType == MapChipType::kBlock) {
		leftHit = true;
	}

	MapChipField::IndexSet rightIndex = mapChipField_->GetMapChipIndexByPosition(cornerPositions[kRightTop]);
	MapChipType rightType = mapChipField_->GetMapChipTypeByIndex(rightIndex.xIndex, rightIndex.yIndex);
	if (rightType == MapChipType::kBlock) {
		rightHit = true;
	}

	if (leftHit || rightHit) {
		// 接触している天井タイルの下端のうち、最も低い(bottomが大きい)位置を採用
		float maxBottom = 0.0f;
		bool initialized = false;
		if (leftHit) {
			Rect r = mapChipField_->GetRectByIndex(leftIndex.xIndex, leftIndex.yIndex);
			maxBottom = r.bottom;
			initialized = true;
		}
		if (rightHit) {
			Rect r = mapChipField_->GetRectByIndex(rightIndex.xIndex, rightIndex.yIndex);
			maxBottom = initialized ? std::max(maxBottom, r.bottom) : r.bottom;
			initialized = true;
		}

		// プレイヤーを天井下端まで押し戻す（半径＋余白ぶん離す)
		float targetY = maxBottom - (kHeight * 0.5f + kBlank);
		worldTransform_.translation_.y = std::min(worldTransform_.translation_.y, targetY);

		// 上方向の速度を止める
		jumpVelocity_ = 0.0f;

		collisionMapInfo.IsCollisionCeiling = true;
	}
}

void Player::CollisionMapDown(CollisionMapInfo& collisionMapInfo) { (void(collisionMapInfo)); }

void Player::CollisionMapLeft(CollisionMapInfo& collisionMapInfo) { (void(collisionMapInfo)); }

void Player::CollisionMapRight(CollisionMapInfo& collisionMapInfo) { (void(collisionMapInfo)); }

Vector3 Player::CornerPisition(const Vector3& center, Corner corner) {

	Vector3 offsetTable[kNumCorners]{
	    {+kWidth / 2.0f, -kHeight / 2.0f, 0.0f}, // 右下
	    {-kWidth / 2.0f, -kHeight / 2.0f, 0.0f}, // 左下
	    {+kWidth / 2.0f, +kHeight / 2.0f, 0.0f}, // 右上
	    {-kWidth / 2.0f, +kHeight / 2.0f, 0.0f}  // 左上
	};

	return center + offsetTable[static_cast<uint32_t>(corner)];
}

void Player::Reflection(CollisionMapInfo& collisionMapInfo) { worldTransform_.translation_ += collisionMapInfo.movement; }
