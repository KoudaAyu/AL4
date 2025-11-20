#include "Player.h"

#include "Enemy.h"
#include "MapChipField.h"

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

Player::Player() {}

Player::~Player() {}

void Player::Initialize(Model* model, Camera* camera, const Vector3& position) {

	textureHandle_ = TextureManager::Load("uvChecker.png");

	assert(model);

	// 引数として受け取ったデータをメンバ関数に記録
	model_ = model;

	/*textureHandle_ = textureHandle;*/

	camera_ = camera;

	// ワールド変換の初期化
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;

	UpdateAABB();
}

// 移動処理
void Player::HandleMovementInput() {

	if (onGround_) {

		if (Input::GetInstance()->PushKey(DIK_RIGHT) || Input::GetInstance()->PushKey(DIK_LEFT)) {
			Vector3 acceleration = {};

			if (Input::GetInstance()->PushKey(DIK_RIGHT)) {
				if (velocity_.x < 0.0f) {
					velocity_.x += (1.0f - kAttenuation);
				}
				if (lrDirection_ != LRDirection::kRight) {
					lrDirection_ = LRDirection::kRight;
					turnFirstRotationY_ = worldTransform_.rotation_.y;
					turnTimer_ = kTimeTurn;
				}
				acceleration.x += kAcceleration;
			} else if (Input::GetInstance()->PushKey(DIK_LEFT)) {
				if (velocity_.x > 0.0f) {
					velocity_.x *= (1.0f - kAttenuation);
				}
				if (lrDirection_ != LRDirection::kLeft) {
					lrDirection_ = LRDirection::kLeft;
					turnFirstRotationY_ = worldTransform_.rotation_.y;
					turnTimer_ = kTimeTurn;
				}
				acceleration.x -= kAcceleration;
			}

			velocity_.x += acceleration.x;
			velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);
		} else {
			// 地上での減衰
			velocity_.x *= (1.0f - kAttenuation);
		}

		if (Input::GetInstance()->PushKey(DIK_UP)) {
			velocity_.y += kJumpAcceleration;
		}

	} else {
		velocity_.y += -kGravityAcceleration;
		velocity_.y = std::max(velocity_.y, -kLimitFallSpeed);
	}
}

void Player::Update() {

	if (!isAlive_) {
		return;
	}

#ifdef _DEBUG
	ImGui::Begin("Debug");
	ImGui::SliderFloat3("velocity", &velocity_.x, -10.0f, 10.0f);
	ImGui::End();
#endif //  _Debug

	if (behaviorRequest_ != Behavior::kUnknown) {
		// 振る舞いを変更
		behavior_ = behaviorRequest_;
		// 各振る舞い開始時の初期化処理
		switch (behavior_) {
		case Behavior::kRoot:
		default:
			BehaviorRootInitialize();
			break;
		case Behavior::kAttack:
			BehaviorAttackInitialize();
			break;
		}
		behaviorRequest_ = Behavior::kUnknown;
	}

	switch (behavior_) {
	case Behavior::kRoot:
	default:
		BehaviorRootUpdate();
		break;
	case Behavior::kAttack:
		BehaviorAttackUpdate();
		break;
	}

	// 1. 移動入力
	HandleMovementInput();

	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		behaviorRequest_ = Behavior::kAttack;
	}

	// 衝突情報を初期化
	CollisionMapInfo collisionInfo;
	// 移動量を加味して現在地を算定するために、現在の速度をcollisionInfoにセット
	collisionInfo.movement_ = velocity_;

	// 2. 移動量を加味して衝突判定する（軸ごとに解決してガタつきを抑制）
	mapChipCollisionCheck(collisionInfo);

	// 3. 判定結果を反映して移動させる
	JudgmentResult(collisionInfo);

	// 4. 天井に接触している場合の処理
	HitCeilingCollision(collisionInfo);

	SwitchingTheGrounding(collisionInfo);

	// 5. 壁に接触している場合の処理
	HitWallCollision(collisionInfo);

	// 6. 壁滑り・壁ジャンプ処理（次フレームの速度に反映される）
	UpdateWallSlide(collisionInfo);
	HandleWallJump(collisionInfo);

	// 7. 旋回制御
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

	UpdateAABB();

	// 8. 行列計算
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();
}

void Player::Draw() {

	// ここに3Dモデルインスタンスの描画処理を記述する
	model_->Draw(worldTransform_, *camera_, textureHandle_);
}

Vector3 Player::CornerPosition(const Vector3& center, Corner corner) {

	Vector3 offsetTable[kNumCorners] = {
	    {-kWidth / 2.0f, +kHeight / 2.0f, 0.0f}, //  左上
	    {+kWidth / 2.0f, +kHeight / 2.0f, 0.0f}, //  右上
	    {-kWidth / 2.0f, -kHeight / 2.0f, 0.0f}, //  左下
	    {+kWidth / 2.0f, -kHeight / 2.0f, 0.0f}, //  右下
	};

	return center + offsetTable[static_cast<uint32_t>(corner)];
}

void Player::mapChipCollisionCheck(CollisionMapInfo& info) {

	// 軸分離解決: まずXのみ、次にXを反映した一時座標でYを解決
	Vector3 originalPos = worldTransform_.translation_;

	// --- X軸 ---
	CollisionMapInfo xInfo; // 局所的に使用
	xInfo.movement_ = {info.movement_.x, 0.0f, 0.0f};
	HandleMapCollisionLeft(xInfo);
	HandleMapCollisionRight(xInfo);
	// Xの結果を適用
	float dx = xInfo.movement_.x;
	info.isWallContact_ = xInfo.isWallContact_;
	info.wallSide_ = xInfo.wallSide_;

	// Xを一時的に反映して、Yの判定に使う
	worldTransform_.translation_.x += dx;

	// --- Y軸 ---
	CollisionMapInfo yInfo;
	yInfo.movement_ = {0.0f, info.movement_.y, 0.0f};
	HandleMapCollisionUp(yInfo);
	HandleMapCollisionDown(yInfo);
	float dy = yInfo.movement_.y;
	info.isCeilingCollision_ = yInfo.isCeilingCollision_;
	info.isLanding_ = yInfo.isLanding_;

	// 一時変更を戻す
	worldTransform_.translation_ = originalPos;

	// 合成結果
	info.movement_ = {dx, dy, 0.0f};
}

// 判定結果を反映して移動
void Player::JudgmentResult(const CollisionMapInfo& info) {
	// 移動
	worldTransform_.translation_ += info.movement_;
}

// 天井に接触している場合
void Player::HitCeilingCollision(CollisionMapInfo& info) {
	if (info.isCeilingCollision_) {
		DebugText::GetInstance()->ConsolePrintf("hit ceiling\n");
		velocity_.y = 0;
	}
}

void Player::HitWallCollision(CollisionMapInfo& info) {
	if (!info.isWallContact_) {
		return;
	}

	// 空中時のみ、壁に向かう速度成分を殺す（壁ジャンプ着地直後の再侵入を防止）
	if (!onGround_) {
		if ((info.wallSide_ == WallSide::kLeft && velocity_.x < 0.0f) || (info.wallSide_ == WallSide::kRight && velocity_.x > 0.0f)) {
			velocity_.x = 0.0f;
		}
	}

	// 地上時は位置のクランプのみに任せ、速度は触らない（ガタつき抑制）
}

void Player::SwitchingTheGrounding(CollisionMapInfo& info) {
	// 自キャラが接地状態
	if (onGround_) {
		// ジャンプ開始
		if (velocity_.y > 0.0f) {
			onGround_ = false;
		} else {
			std::array<Vector3, kNumCorners> positionNew;
			for (uint32_t i = 0; i < positionNew.size(); ++i) {
				positionNew[i] = CornerPosition(worldTransform_.translation_ + info.movement_, static_cast<Corner>(i));
			}

			MapChipType mapChipType;

			bool hit = false;

			IndexSet indexSet;

			indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftTop]);

			constexpr float kGroundCheckOffsetY = 0.1f; // わずかに下を見る

			// 左下点
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftBottom] - Vector3{0.0f, kGroundCheckOffsetY, 0.0f});
			mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
			if (mapChipType == MapChipType::kBlock) {
				hit = true;
			}

			// 右下点
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightBottom] - Vector3{0.0f, kGroundCheckOffsetY, 0.0f});
			mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
			if (mapChipType == MapChipType::kBlock) {
				hit = true;
			}
			// 落下開始
			if (!hit) {
				onGround_ = false;
			}
		}

	} else {

		if (info.isLanding_) {
			// 着地状態に切り替える
			onGround_ = true;

			// 着地時にX座標を減衰
			velocity_.x *= (1.0f - kAttenuationLanding);

			// Y座標をゼロにする
			velocity_.y = 0.0f;
		}
	}
}

// マップチップ衝突判定（上方向）
void Player::HandleMapCollisionUp(CollisionMapInfo& info) {

	std::array<Vector3, kNumCorners> positionNew;
	for (uint32_t i = 0; i < positionNew.size(); ++i) {
		positionNew[i] = CornerPosition(worldTransform_.translation_ + info.movement_, static_cast<Corner>(i));
	}
	if (info.movement_.y <= 0) {
		return;
	}

	MapChipType mapChipType;
	// 真上の当たり判定を行う
	bool hit = false;
	// 左上点の座標
	IndexSet indexSet;

	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftTop]);

	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	// 右上点の座標
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	// 衝突している場合
	if (hit) {

		IndexSet indexSetNow;
		// 現在の左上点のタイルと比較して、上方向への遷移を検出
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(worldTransform_.translation_, kLeftTop));
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftTop]);
		if (indexSetNow.yIndex != indexSet.yIndex) {

			// めり込み先ブロックの範囲矩形
			Rects rects = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);

			info.movement_.y = std::max(0.0f, rects.bottom - worldTransform_.translation_.y - (kHeight * 0.5f + kBlank));

			// 天井に当たったことを記録する
			info.isCeilingCollision_ = true;
		}
	}
}

void Player::HandleMapCollisionDown(CollisionMapInfo& info) {

	std::array<Vector3, kNumCorners> positionNew;
	for (uint32_t i = 0; i < positionNew.size(); ++i) {
		positionNew[i] = CornerPosition(worldTransform_.translation_ + info.movement_, static_cast<Corner>(i));
	}

	// 下降あり
	if (info.movement_.y >= 0) {
		return;
	}
	MapChipType mapChipType;

	// 真下の当たり判定を行う
	bool hit = false;

	IndexSet indexSet;

	// 左下点の座標
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	// 右下点の座標
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftBottom]);

		// 現在の左下点のタイルと比較して、下方向への遷移を検出
		IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(worldTransform_.translation_, kLeftBottom));

		if (indexSetNow.yIndex != indexSet.yIndex) {

			// めり込み先ブロックの範囲矩形
			Rects rects = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);

			info.movement_.y = std::min(0.0f, rects.top - worldTransform_.translation_.y + (kHeight * 0.5f + kBlank));

			// 地面に当たったことを記録する
			info.isLanding_ = true;
		}
	}
}

void Player::HandleMapCollisionLeft(CollisionMapInfo& info) {
	std::array<Vector3, kNumCorners> positionNew;
	for (uint32_t i = 0; i < positionNew.size(); ++i) {
		positionNew[i] = CornerPosition(worldTransform_.translation_ + info.movement_, static_cast<Corner>(i));
	}
	MapChipType mapChipType;

	bool hit = false;

	if (info.movement_.x >= 0.0f) {
		return;
	}

	IndexSet indexSet;

	// 左上点の座標
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	// 左下点の座標
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftTop]);

		IndexSet indexSetNow;
		// 現在の左上点のタイルと比較して、左壁への遷移を検出
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(worldTransform_.translation_, kLeftTop));
		if (indexSetNow.xIndex != indexSet.xIndex) {

			// めり込み先ブロックの範囲矩形
			Rects rects = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);

			// 左壁の許容移動量（壁外側に押し戻さないクランプ）
			float dxAllowed = (rects.right + kBlank) - (worldTransform_.translation_.x - kWidth * 0.5f); // 目標位置をブロックの右からkBlank分足す
			info.movement_.x = std::min(0.0f, std::max(info.movement_.x, dxAllowed));

			info.isWallContact_ = true;
			info.wallSide_ = WallSide::kLeft;
		}
	}
}

void Player::HandleMapCollisionRight(CollisionMapInfo& info) {

	std::array<Vector3, kNumCorners> positionNew;
	for (uint32_t i = 0; i < positionNew.size(); ++i) {
		positionNew[i] = CornerPosition(worldTransform_.translation_ + info.movement_, static_cast<Corner>(i));
	}
	MapChipType mapChipType;

	bool hit = false;

	if (info.movement_.x <= 0.0f) {
		return;
	}

	IndexSet indexSet;

	// 右上点の座標
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	// 右下点の座標
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightTop]);

		IndexSet indexSetNow;
		// 現在の右上点のインデックスと比較して、右壁への遷移を検出
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(worldTransform_.translation_, kRightTop));
		if (indexSetNow.xIndex != indexSet.xIndex) {
			// めり込み先ブロックの範囲矩形
			Rects rects = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);

			// 右壁の許容移動量（壁外側に押し戻さないクランプ）
			float dxAllowed = (rects.left - kBlank) - (worldTransform_.translation_.x + kWidth * 0.5f); // 目標位置をブロックの左からkBlank分引く
			info.movement_.x = std::max(0.0f, std::min(info.movement_.x, dxAllowed));

			info.isWallContact_ = true;
			info.wallSide_ = WallSide::kRight;
		}
	}
}

void Player::UpdateWallSlide(const CollisionMapInfo& info) {
	// クールダウン更新
	if (wallJumpCooldown_ > 0.0f) {
		wallJumpCooldown_ -= 1.0f / 60.0f;
		if (wallJumpCooldown_ < 0.0f) {
			wallJumpCooldown_ = 0.0f;
		}
	}

	isWallSliding_ = false;
	if (onGround_) {
		return;
	}

	if (info.isWallContact_ && velocity_.y < 0.0f) {
		bool pressingTowardWall = false;
		switch (info.wallSide_) {
		case WallSide::kLeft:
			pressingTowardWall = Input::GetInstance()->PushKey(DIK_LEFT);
			break;
		case WallSide::kRight:
			pressingTowardWall = Input::GetInstance()->PushKey(DIK_RIGHT);
			break;
		default:
			break;
		}
		if (pressingTowardWall) {
			isWallSliding_ = true;
			// 落下速度を制限
			velocity_.y = std::max(velocity_.y, -kWallSlideMaxFallSpeed);
		}
	}
}

void Player::HandleWallJump(const CollisionMapInfo& info) {
	if (onGround_) {
		return;
	}

	bool canWallJump = (isWallSliding_ || info.isWallContact_);
	if (!canWallJump) {
		return;
	}

	bool jumpPressed = Input::GetInstance()->PushKey(DIK_UP);
	if (jumpPressed && wallJumpCooldown_ <= 0.0f) {
		// 反対方向へ跳ねる
		if (info.wallSide_ == WallSide::kLeft) {
			velocity_.x = +kWallJumpHorizontalSpeed;
			lrDirection_ = LRDirection::kRight;
		} else if (info.wallSide_ == WallSide::kRight) {
			velocity_.x = -kWallJumpHorizontalSpeed;
			lrDirection_ = LRDirection::kLeft;
		}
		velocity_.y = kWallJumpVerticalSpeed;
		isWallSliding_ = false;

		// 旋回演出
		turnFirstRotationY_ = worldTransform_.rotation_.y;
		turnTimer_ = kTimeTurn;

		// 連続発動防止
		wallJumpCooldown_ = kWallJumpCooldownTime;
	}
}

void Player::OnCollision(Enemy* enemy) {
	(void)enemy;
	isAlive_ = false;
}

void Player::UpdateAABB() {
	// プレイヤーと同等サイズの簡易AABB（必要なら調整）

	static constexpr float kDepth = 0.8f * 2.0f;

	Vector3 center = worldTransform_.translation_;
	Vector3 half = {kWidth * 0.5f, kHeight * 0.5f, kDepth * 0.5f};

	// 回転は無視して軸整列AABBを更新（必要ならメッシュ頂点から算出実装へ拡張）
	aabb_.min = {center.x - half.x, center.y - half.y, center.z - half.z};
	aabb_.max = {center.x + half.x, center.y + half.y, center.z + half.z};
}

void Player::BehaviorRootInitialize() {}

void Player::BehaviorAttackInitialize() {
//カウンターの初期化
	attackParameter_ = 0;
}

void Player::BehaviorRootUpdate() {}

void Player::BehaviorAttackUpdate() {

	// 攻撃動作
	attackParameter_++;

	if (attackParameter_ > kAttackDuration) {
		behaviorRequest_ = Behavior::kRoot;
	}

	worldTransform_.rotation_.x += 0.4f;

}
