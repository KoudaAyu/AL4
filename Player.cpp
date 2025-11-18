#include "Player.h"

#include "KeyInput.h"
#include "MapChipField.h"
#include "MathUtl.h"
#include <algorithm>
#include <cassert>
#include <numbers>
#include <cfloat>

using namespace KamataEngine;

//=============================================================================
// ユーティリティ
//=============================================================================

KamataEngine::Vector3 Player::CornerPisition(const KamataEngine::Vector3& center, Corner corner) {

	KamataEngine::Vector3 offsetTable[kNumCorners]{
	    {+kWidth / 2.0f, -kHeight / 2.0f, 0.0f}, // 右下
	    {-kWidth / 2.0f, -kHeight / 2.0f, 0.0f}, // 左下
	    {+kWidth / 2.0f, +kHeight / 2.0f, 0.0f}, // 右上
	    {-kWidth / 2.0f, +kHeight / 2.0f, 0.0f}  // 左上
	};

	return center + offsetTable[static_cast<uint32_t>(corner)];
}

void Player::Reflection(CollisionMapInfo& collisionMapInfo) {
	// この関数は使用しない
	(void(collisionMapInfo));
}

//=============================================================================
// メイン処理
//=============================================================================

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

	// 1. 速度の計算（Move/Jump）
	Move();

	// 2. 衝突判定と速度の調整
	CollisionMapInfo collisionpMaInfo;
	collisionpMaInfo.movement = velocity_;
	CollisionMap(collisionpMaInfo);

	// 3. 座標の更新
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

void Player::Move() {
	// XBoxコントローラーの左スティック入力取得
	KamataEngine::Vector2 lStick = KeyInput::GetInstance()->GetLStick();

	// スティックのデッドゾーン処理（必要に応じて）
	if (fabs(lStick.x) > 0.1f) {
		velocity_.x += lStick.x * kAcceleration;
		velocity_.x = std::clamp(velocity_.x, -kLimitSpeed, kLimitSpeed);
	}

	// キー入力による移動
	if (Input::GetInstance()->PushKey(DIK_RIGHT) || Input::GetInstance()->PushKey(DIK_LEFT)) {
		KamataEngine::Vector3 acceleration = {};
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

	// worldTransform_.translation_ += velocity_; // 削除: Updateの最後で実行

	Jump();
}

void Player::Jump() {

	// ジャンプ入力
	if ((KeyInput::GetInstance()->TriggerPadButton(XINPUT_GAMEPAD_A) || Input::GetInstance()->TriggerKey(DIK_SPACE)) && jumpCount_ < kMaxJumpCount) {
		isJump_ = true;
		velocity_.y = kJumpVelocity;
		jumpCount_++;
	}

	// 重力処理
	if (isJump_) {
		velocity_.y -= kGravity;
		// worldTransform_.translation_.y += jumpVelocity_; // 削除: Updateの最後で実行
	}
}

//=============================================================================
// 衝突処理
//=============================================================================

void Player::CollisionMap(CollisionMapInfo& collisionMapInfo) {

	// 垂直方向の処理を先に実行
	CollisionMapUp(collisionMapInfo);
	CollisionMapDown(collisionMapInfo);

	// 水平方向の処理を実行
	CollisionMapLeft(collisionMapInfo);
	CollisionMapRight(collisionMapInfo);
}

void Player::CollisionMapUp(CollisionMapInfo& collisionMapInfo) {

	// 上方向へ移動していないなら判定不要
	if (velocity_.y <= 0.0f) {
		return;
	}

	// 予測される移動後の位置
	KamataEngine::Vector3 predictedPosition = worldTransform_.translation_ + velocity_;

	// プレイヤーの4つのコーナーの位置を計算（予測位置で判定）
	std::array<Vector3, kNumCorners> cornerPositions{};
	for (uint32_t i = 0; i < cornerPositions.size(); ++i) {
		cornerPositions[i] = CornerPisition(predictedPosition, static_cast<Corner>(i));
	}

	// 真上の当たり判定を行う（左右の上コーナー）
	bool leftHit = false;
	bool rightHit = false;

	// 左上の角
	MapChipField::IndexSet leftIndex = mapChipField_->GetMapChipIndexByPosition(cornerPositions[kLeftTop]);
	MapChipType leftType = mapChipField_->GetMapChipTypeByIndex(leftIndex.xIndex, leftIndex.yIndex);
	if (leftType == MapChipType::kBlock) {
		leftHit = true;
	}

	// 右上の角
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

		// プレイヤーの上端が天井下端に触れるように Y 速度を調整
		float playerTopY = worldTransform_.translation_.y + kHeight * 0.5f;
		float targetTopY = maxBottom - kBlank;

		// 調整後の Y 速度 (天井にめり込ませないように調整)
		velocity_.y = std::min(0.0f, targetTopY - playerTopY);

		// Y座標を直接修正しておくことで、velocity_が大きすぎる場合の保険とする
		worldTransform_.translation_.y = targetTopY - kHeight * 0.5f;

		collisionMapInfo.IsCollisionCeiling = true;
	}
}

void Player::CollisionMapDown(CollisionMapInfo& collisionMapInfo) {
	if (!mapChipField_) {
		return;
	}

	// 下降(または静止)時のみ着地判定。上昇中は床チェック不要
	if (velocity_.y > 0.0f) {
		return;
	}

	// 次のX位置で壁に当たるか先にチェック
	bool wallNext = false;
	KamataEngine::Vector3 predictedPosition = worldTransform_.translation_ + velocity_;
	if (velocity_.x > 0.0f) {
		Vector3 rb = CornerPisition(predictedPosition, kRightBottom);
		Vector3 rt = CornerPisition(predictedPosition, kRightTop);
		MapChipField::IndexSet rbi = mapChipField_->GetMapChipIndexByPosition(rb);
		MapChipField::IndexSet rti = mapChipField_->GetMapChipIndexByPosition(rt);
		MapChipType t1 = mapChipField_->GetMapChipTypeByIndex(rbi.xIndex, rbi.yIndex);
		MapChipType t2 = mapChipField_->GetMapChipTypeByIndex(rti.xIndex, rti.yIndex);
		wallNext = (t1 == MapChipType::kBlock) || (t2 == MapChipType::kBlock);
	} else if (velocity_.x < 0.0f) {
		Vector3 lb = CornerPisition(predictedPosition, kLeftBottom);
		Vector3 lt = CornerPisition(predictedPosition, kLeftTop);
		MapChipField::IndexSet lbi = mapChipField_->GetMapChipIndexByPosition(lb);
		MapChipField::IndexSet lti = mapChipField_->GetMapChipIndexByPosition(lt);
		MapChipType t1 = mapChipField_->GetMapChipTypeByIndex(lbi.xIndex, lbi.yIndex);
		MapChipType t2 = mapChipField_->GetMapChipTypeByIndex(lti.xIndex, lti.yIndex);
		wallNext = (t1 == MapChipType::kBlock) || (t2 == MapChipType::kBlock);
	}

	// 高速落下対策: Y方向にスイープして複数サンプル
	const float path = fabsf(velocity_.y);
	const float stepLen = std::max(0.1f, kHeight * 0.25f);
	int steps = static_cast<int>(std::ceil(path / stepLen));
	steps = std::max(1, steps);

	bool leftSupportCur = false, rightSupportCur = false;
	bool leftSupportNext = false, rightSupportNext = false;
	float floorTopCur = -FLT_MAX;
	float floorTopNext = -FLT_MAX;

	for (int s = 1; s <= steps; ++s) {
		float t = static_cast<float>(s) / static_cast<float>(steps);
		float sampleCenterY = worldTransform_.translation_.y + velocity_.y * t;

		// 現在Xでのサンプル
		KamataEngine::Vector3 leftProbeCur = CornerPisition({worldTransform_.translation_.x, sampleCenterY, worldTransform_.translation_.z}, kLeftBottom);
		KamataEngine::Vector3 rightProbeCur = CornerPisition({worldTransform_.translation_.x, sampleCenterY, worldTransform_.translation_.z}, kRightBottom);
		leftProbeCur.y -= kGroundProbeEps; rightProbeCur.y -= kGroundProbeEps;
		MapChipField::IndexSet liCur = mapChipField_->GetMapChipIndexByPosition(leftProbeCur);
		MapChipField::IndexSet riCur = mapChipField_->GetMapChipIndexByPosition(rightProbeCur);
		MapChipType ltCur = mapChipField_->GetMapChipTypeByIndex(liCur.xIndex, liCur.yIndex);
		MapChipType rtCur = mapChipField_->GetMapChipTypeByIndex(riCur.xIndex, riCur.yIndex);
		if (ltCur == MapChipType::kBlock) { leftSupportCur = true; Rect r = mapChipField_->GetRectByIndex(liCur.xIndex, liCur.yIndex); floorTopCur = std::max(floorTopCur, r.top); }
		if (rtCur == MapChipType::kBlock) { rightSupportCur = true; Rect r = mapChipField_->GetRectByIndex(riCur.xIndex, riCur.yIndex); floorTopCur = std::max(floorTopCur, r.top); }

		// 次Xでのサンプル（次に壁がない場合のみ）
		if (!wallNext) {
			KamataEngine::Vector3 leftProbeNext = CornerPisition({predictedPosition.x, sampleCenterY, worldTransform_.translation_.z}, kLeftBottom);
			KamataEngine::Vector3 rightProbeNext = CornerPisition({predictedPosition.x, sampleCenterY, worldTransform_.translation_.z}, kRightBottom);
			leftProbeNext.y -= kGroundProbeEps; rightProbeNext.y -= kGroundProbeEps;
			MapChipField::IndexSet liNext = mapChipField_->GetMapChipIndexByPosition(leftProbeNext);
			MapChipField::IndexSet riNext = mapChipField_->GetMapChipIndexByPosition(rightProbeNext);
			MapChipType ltNext = mapChipField_->GetMapChipTypeByIndex(liNext.xIndex, liNext.yIndex);
			MapChipType rtNext = mapChipField_->GetMapChipTypeByIndex(riNext.xIndex, riNext.yIndex);
			if (ltNext == MapChipType::kBlock) { leftSupportNext = true; Rect r = mapChipField_->GetRectByIndex(liNext.xIndex, liNext.yIndex); floorTopNext = std::max(floorTopNext, r.top); }
			if (rtNext == MapChipType::kBlock) { rightSupportNext = true; Rect r = mapChipField_->GetRectByIndex(riNext.xIndex, riNext.yIndex); floorTopNext = std::max(floorTopNext, r.top); }
		}
	}

	bool hasSupportCur = leftSupportCur || rightSupportCur;
	bool hasSupportNext = leftSupportNext || rightSupportNext;

	// デフォルトは現在Xでの支えを採用
	bool useCurrent = true;
	// 壁で横移動が塞がれていない場合、次Xでの支えに置き換え（早めに落下できる）
	if (!wallNext) {
		useCurrent = hasSupportCur; // いま支えがあれば維持、なければ次Xを見る
		if (!hasSupportCur && hasSupportNext) { useCurrent = false; }
		if (!hasSupportCur && !hasSupportNext) { useCurrent = false; }
	}
	// 壁で塞がれている場合でも、いまの支えがなくなったら落下許可
	if (wallNext && !hasSupportCur) {
		useCurrent = false;
	}

	bool hasSupport = useCurrent ? hasSupportCur : hasSupportNext;
	float floorTop = useCurrent ? floorTopCur : floorTopNext;

	// 支えが無ければ落下フラグ（このフレームで速度は変えない。重力は次フレームから）
	if (!hasSupport) {
		isJump_ = true;
	}

	if (hasSupport) {
		float desiredY = floorTop + (kHeight * 0.5f) + kGroundBlankDown;
		float playerBottomY = worldTransform_.translation_.y - (kHeight * 0.5f);
		if (playerBottomY + velocity_.y <= floorTop + kGroundBlankDown) {
			worldTransform_.translation_.y = desiredY;
			velocity_.y = 0.0f;
			isJump_ = false;
			jumpCount_ = 0;
			collisionMapInfo.IsLanding = true;
		}
	}
}

void Player::CollisionMapLeft(CollisionMapInfo& collisionMapInfo) {
	// 左壁当たり判定（右壁と対称）
	if (!mapChipField_ || velocity_.x >= 0.0f) {
		return;
	}

	// 予測される移動後の位置
	KamataEngine::Vector3 predictedPosition = worldTransform_.translation_ + velocity_;

	// 予測位置の4コーナー取得
	std::array<Vector3, kNumCorners> cornerPositions{};
	for (uint32_t i = 0; i < cornerPositions.size(); ++i) {
		cornerPositions[i] = CornerPisition(predictedPosition, static_cast<Corner>(i));
	}

	// 左側上下の判定点 (予測位置)
	Vector3 leftBottomProbe = cornerPositions[kLeftBottom];
	Vector3 leftTopProbe = cornerPositions[kLeftTop];

	bool bottomHit = false;
	bool topHit = false;

	MapChipField::IndexSet bottomIndex = mapChipField_->GetMapChipIndexByPosition(leftBottomProbe);
	MapChipType bottomType = mapChipField_->GetMapChipTypeByIndex(bottomIndex.xIndex, bottomIndex.yIndex);
	if (bottomType == MapChipType::kBlock) {
		bottomHit = true;
	}

	MapChipField::IndexSet topIndex = mapChipField_->GetMapChipIndexByPosition(leftTopProbe);
	MapChipType topType = mapChipField_->GetMapChipTypeByIndex(topIndex.xIndex, topIndex.yIndex);
	if (topType == MapChipType::kBlock) {
		topHit = true;
	}

	if (bottomHit || topHit) {
		// 衝突しているタイル群の最も右の right を取得
		float maxRight = 0.0f;
		bool initialized = false;
		if (bottomHit) {
			Rect r = mapChipField_->GetRectByIndex(bottomIndex.xIndex, bottomIndex.yIndex);
			maxRight = r.right;
			initialized = true;
		}
		if (topHit) {
			Rect r = mapChipField_->GetRectByIndex(topIndex.xIndex, topIndex.yIndex);
			maxRight = initialized ? std::max(maxRight, r.right) : r.right;
		}

		// 押し出し後の目標X中心座標
		float targetX = maxRight + kWidth * 0.5f + kWallBlank;

		// X座標を直接修正しておく（速度が大きすぎる場合の安全策）
		worldTransform_.translation_.x = targetX;

		// 水平方向速度停止（右壁と同様の処理に統一）
		velocity_.x = 0.0f;
		collisionMapInfo.IsCollisionWall = true;
		collisionMapInfo.IsLanding = false; // 左でも床補正を抑制
	}
}

void Player::CollisionMapRight(CollisionMapInfo& collisionMapInfo) {
	// 右壁当たり判定（左壁と対称）
	if (!mapChipField_ || velocity_.x <= 0.0f) {
		return;
	}

	// 予測される移動後の位置
	KamataEngine::Vector3 predictedPosition = worldTransform_.translation_ + velocity_;

	// 予測位置の4コーナー取得
	std::array<Vector3, kNumCorners> cornerPositions{};
	for (uint32_t i = 0; i < cornerPositions.size(); ++i) {
		cornerPositions[i] = CornerPisition(predictedPosition, static_cast<Corner>(i));
	}

	// 右側上下の判定点 (予測位置)
	Vector3 rightBottomProbe = cornerPositions[kRightBottom];
	Vector3 rightTopProbe = cornerPositions[kRightTop];

	bool bottomHit = false;
	bool topHit = false;

	MapChipField::IndexSet bottomIndex = mapChipField_->GetMapChipIndexByPosition(rightBottomProbe);
	MapChipType bottomType = mapChipField_->GetMapChipTypeByIndex(bottomIndex.xIndex, bottomIndex.yIndex);
	if (bottomType == MapChipType::kBlock) {
		bottomHit = true;
	}

	MapChipField::IndexSet topIndex = mapChipField_->GetMapChipIndexByPosition(rightTopProbe);
	MapChipType topType = mapChipField_->GetMapChipTypeByIndex(topIndex.xIndex, topIndex.yIndex);
	if (topType == MapChipType::kBlock) {
		topHit = true;
	}

	if (bottomHit || topHit) {
		// 衝突しているタイル群の最も左の left を取得
		float minLeft = 0.0f;
		bool initialized = false;
		if (bottomHit) {
			Rect r = mapChipField_->GetRectByIndex(bottomIndex.xIndex, bottomIndex.yIndex);
			minLeft = r.left;
			initialized = true;
		}
		if (topHit) {
			Rect r = mapChipField_->GetRectByIndex(topIndex.xIndex, topIndex.yIndex);
			minLeft = initialized ? std::min(minLeft, r.left) : r.left;
		}

		// 押し出し後の目標X中心座標
		float targetX = minLeft - kWidth * 0.5f - kWallBlank;

		// X座標を直接修正しておく（速度が大きすぎる場合の安全策）
		worldTransform_.translation_.x = targetX;

		// 水平方向速度停止
		velocity_.x = 0.0f;
		collisionMapInfo.IsCollisionWall = true;
		collisionMapInfo.IsLanding = false;
	}
}
