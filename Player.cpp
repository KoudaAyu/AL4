#include "Player.h"

#include "CameraController.h"
#include "Enemy.h"
#include "MapChipField.h"

#include <Windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#include <Xinput.h>
#pragma comment(lib, "xinput.lib")

using namespace KamataEngine;

namespace {

struct RumbleState {
	bool active = false;
	ULONGLONG endTick = 0;
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
	XINPUT_VIBRATION vib{};
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

static float NormalizeLeftStickX(SHORT rawValue) {

	const float denom = 32767.0f;
	float v = 0.0f;
	if (rawValue == -32768) {
		v = -1.0f;
	} else {
		v = static_cast<float>(rawValue) / denom;
	}
	v = std::clamp(v, -1.0f, 1.0f);
	const float deadzone = static_cast<float>(XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) / denom;
	if (std::fabs(v) < deadzone) {
		return 0.0f;
	}
	return v;
}

static float NormalizeLeftStickY(SHORT rawValue) {

	const float denom = 32767.0f;
	float v = 0.0f;
	if (rawValue == -32768) {
		v = -1.0f;
	} else {
		v = static_cast<float>(rawValue) / denom;
	}
	v = std::clamp(v, -1.0f, 1.0f);
	const float deadzone = static_cast<float>(XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) / denom;
	if (std::fabs(v) < deadzone) {
		return 0.0f;
	}
	return v;
}

static float GetHorizontalInputIntensity(float stickX, bool keyRight, bool keyLeft) {

	if (keyRight)
		return 1.0f;
	if (keyLeft)
		return 1.0f;

	return std::clamp(std::fabs(stickX), 0.0f, 1.0f);
}

static bool IsPressingTowardWall(const XINPUT_STATE& state, WallSide side) {
	float stickX = NormalizeLeftStickX(state.Gamepad.sThumbLX);

	bool keyLeft = Input::GetInstance()->PushKey(DIK_LEFT) || Input::GetInstance()->PushKey(DIK_A);
	bool keyRight = Input::GetInstance()->PushKey(DIK_RIGHT) || Input::GetInstance()->PushKey(DIK_D);

	switch (side) {
	case WallSide::kLeft:
		return keyLeft || (stickX < -0.2f);
	case WallSide::kRight:
		return keyRight || (stickX > 0.2f);
	default:
		return false;
	}
}

} 

Player::Player() {}

Player::~Player() {

	if (ownsModel_ && model_) {
		delete model_;
		model_ = nullptr;
	}
}

void Player::Initialize(Camera* camera, const Vector3& position) {

	textureHandle_ = TextureManager::Load("attack_effect/attack_effect.png");

	model_ = Model::CreateFromOBJ("Player", true);
	ownsModel_ = true;

	assert(model_);

	attackModel_ = Model::CreateFromOBJ("attack_effect", true);

	camera_ = camera;

	// ワールド変換の初期化
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
	worldTransform_.scale_ = {0.3f, 0.3f, 0.3f};

	// store base Y scale for crouch restore
	baseScaleY_ = worldTransform_.scale_.y;

	// 攻撃エフェクトの初期化（プレイヤーと同じ回転・位置、スケールは拡大）
	attackWorldTransform_.Initialize();
	attackWorldTransform_.scale_ = {worldTransform_.scale_.x * kAttackEffectScale,
							 worldTransform_.scale_.y * kAttackEffectScale,
							 worldTransform_.scale_.z * kAttackEffectScale};
	attackWorldTransform_.rotation_ = worldTransform_.rotation_;
	attackWorldTransform_.translation_ = worldTransform_.translation_;

	UpdateAABB();

	
	hp_ = kMaxHP;
	isAlive_ = true;
	isDying_ = false;

	seSlidingDecisionDataHandle_ = Audio::GetInstance()->LoadWave("Audio/SE/Sliding.wav");
	seJumpDecisionDataHandle_ = Audio::GetInstance()->LoadWave("Audio/SE/Jump.wav");
	seDamageSoundHandle_ = Audio::GetInstance()->LoadWave("Audio/SE/Damage.wav");
	seAttackSoundHandle_ = Audio::GetInstance()->LoadWave("Audio/SE/Attack.wav");
}

// 移動処理
void Player::HandleMovementInput() {

    Input::GetInstance()->GetJoystickState(0, state);

    if (isDodging_) {

        if (!onGround_) {
            velocity_.y += -kGravityAcceleration;
            velocity_.y = std::max(velocity_.y, -kLimitFallSpeed);
        }
        return;
    }

	//緊急回避用キー  
    bool qTriggered = Input::GetInstance()->TriggerKey(DIK_Q);

	static bool prevLeftTriggerPressed = false;
    bool leftTriggerPressed = (state.Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
    bool leftTriggerRising = leftTriggerPressed && !prevLeftTriggerPressed;
    prevLeftTriggerPressed = leftTriggerPressed;
    
  
    bool dodgeTriggered = qTriggered || leftTriggerRising;

  
    if (dodgeTriggered && !isDodging_ && dodgeCooldown_ <= 0.0f && behavior_ != Behavior::kAttack && !isDying_) {
        float dir = (lrDirection_ == LRDirection::kRight) ? 1.0f : -1.0f;
        velocity_.x = dir * kDodgeSpeed;
        isDodging_ = true;
        dodgeTimer_ = kDodgeDuration;
        dodgeCooldown_ = kDodgeCooldownTime;
        // play sliding sound
		if (seSlidingDecisionDataHandle_ != 0u) {
			Audio::GetInstance()->PlayWave(seSlidingDecisionDataHandle_, false, 1.0f);
		}
		
        if (!isCrouching_) {
            isCrouching_ = true;
            crouchForcedByDodge_ = true;
          
            float oldHalfHeight = kHeight * 0.5f * worldTransform_.scale_.y;
            float newScaleY = baseScaleY_ * kCrouchVisualScale;
            float newHalfHeight = kHeight * 0.5f * newScaleY;
            
            worldTransform_.scale_.y = newScaleY;
            worldTransform_.translation_.y += (newHalfHeight - oldHalfHeight);
            UpdateAABB();
        }
        if (cameraController_)
            cameraController_->StartShake(0.5f, 0.12f);
    }

    float stickX = NormalizeLeftStickX(state.Gamepad.sThumbLX);

    bool keyRight = Input::GetInstance()->PushKey(DIK_RIGHT) || Input::GetInstance()->PushKey(DIK_D);
    bool keyLeft = Input::GetInstance()->PushKey(DIK_LEFT) || Input::GetInstance()->PushKey(DIK_A);

    bool moveRight = keyRight || (stickX > 0.0f);
    bool moveLeft = keyLeft || (stickX < 0.0f);

    // 地面の摩擦係数を取得（デフォルトは1.0f相当）。利用可能なら取得
    float groundFriction = 0.9f;
    if (mapChipField_) {
        Vector3 samplePos = worldTransform_.translation_ + Vector3{0.0f, -(kHeight * 0.5f) - 0.02f, 0.0f};
        groundFriction = mapChipField_->GetFrictionCoefficientByPosition(samplePos);
        onIce_ = (groundFriction < 0.1f);
    }

    // ハシゴ判定: プレイヤー中心がハシゴタイルと重なるならハシゴ状態
    bool ladderHere = false;
    if (mapChipField_) {
        IndexSet idxCenter = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_);
        MapChipType centerType = mapChipField_->GetMapChipTypeByIndex(idxCenter.xIndex, idxCenter.yIndex);
        if (centerType == MapChipType::kLadder) ladderHere = true;
        // 足元位置も確認し、少しずれていてもハシゴを掴めるようにする
        Vector3 feetSample = worldTransform_.translation_ + Vector3{0.0f, - (kHeight * 0.5f) + 0.1f, 0.0f};
        IndexSet idxFeet = mapChipField_->GetMapChipIndexSetByPosition(feetSample);
        MapChipType feetType = mapChipField_->GetMapChipTypeByIndex(idxFeet.xIndex, idxFeet.yIndex);
        if (feetType == MapChipType::kLadder) ladderHere = true;
    }

    // ハシゴ昇降入力
    // include gamepad left stick vertical for climbing
    float stickY_for_climb = NormalizeLeftStickY(state.Gamepad.sThumbLY);
    bool climbUp = Input::GetInstance()->PushKey(DIK_W) || (stickY_for_climb > 0.2f);
    bool climbDown = Input::GetInstance()->PushKey(DIK_S) || (stickY_for_climb < -0.2f);

    if (ladderHere && (climbUp || climbDown)) {
        // ハシゴ状態へ移行
        onLadder_ = true;
    }

    // ハシゴ上にいる場合、W/Sでの上下移動を処理し、重力や地上摩擦は無視
    if (onLadder_) {
        // 既存の縦方向速度をキャンセルし、昇降を適用
        velocity_.y = 0.0f;
        // allow analog stick control for smooth climbing
        float stickY = NormalizeLeftStickY(state.Gamepad.sThumbLY);
        if (std::fabs(stickY) > 0.01f) {
            // stickY is [-1,1], positive means up
            velocity_.y = stickY * kClimbSpeed;
        } else if (climbUp) {
            velocity_.y = kClimbSpeed;
        } else if (climbDown) {
            velocity_.y = -kClimbSpeed;
        } else {
            velocity_.y = 0.0f;
        }

        // ハシゴ上では、強い減衰をかけつつ限定的に横方向の操作を許可
        // ハシゴ昇降中も、ADキーまたはスティックによる限定的な横移動を許可
        stickX = NormalizeLeftStickX(state.Gamepad.sThumbLX);
         float horizInput = 0.0f;
         // キーボード入力を優先的に最大値とし、そうでなければスティック値で滑らかに
         if (keyRight)
             horizInput = 1.0f;
         else if (keyLeft)
             horizInput = -1.0f;
         else
             horizInput = stickX; // stickX は [-1,1] の範囲

        // Update facing direction on ladder when player provides horizontal input
        if (horizInput > 0.01f) {
            if (lrDirection_ != LRDirection::kRight) {
                lrDirection_ = LRDirection::kRight;
                turnFirstRotationY_ = worldTransform_.rotation_.y;
                turnTimer_ = kTimeTurn;
            }
        } else if (horizInput < -0.01f) {
            if (lrDirection_ != LRDirection::kLeft) {
                lrDirection_ = LRDirection::kLeft;
                turnFirstRotationY_ = worldTransform_.rotation_.y;
                turnTimer_ = kTimeTurn;
            }
        }

        // ハシゴ上での目標横速度
        float targetVx = std::clamp(horizInput, -1.0f, 1.0f) * kLadderHorizontalSpeed;
        // 目標へ滑らかに補間（係数が小さいほど穏やか）
        velocity_.x += (targetVx - velocity_.x) * kLadderHorizontalAccel;
        // 入力がないときの微小減衰で、ゆっくり中央へ収束
        if (std::fabs(horizInput) < 0.01f) {
            velocity_.x *= 0.95f;
        }

        // プレイヤーがハシゴタイルから離れたら、ハシゴ状態を離脱することを検討
        if (!ladderHere) {
            if (mapChipField_) {
                // プレイヤーの頭上ブロックを探索
                Vector3 probePos = worldTransform_.translation_ + Vector3{0.0f, kHeight * 0.5f + 0.02f, 0.0f};
                IndexSet probeIdx = mapChipField_->GetMapChipIndexSetByPosition(probePos);
                MapChipType aboveType = mapChipField_->GetMapChipTypeByIndex(probeIdx.xIndex, probeIdx.yIndex);
                if (aboveType == MapChipType::kBlock || aboveType == MapChipType::kIce) {
                    // 許容誤差内ならプレイヤーの足元をブロック上面にスナップして、Wを離しても落下しないよう調整
                    Rects rect = mapChipField_->GetRectByIndex(probeIdx.xIndex, probeIdx.yIndex);
                    float desiredY = rect.top + (kHeight * 0.5f); // 足元が rect.top に一致するような translation_.y
                    float tolerance = 0.5f; // わずかなオーバーシュートを許容
                    if (worldTransform_.translation_.y <= desiredY + tolerance) {
                        worldTransform_.translation_.y = desiredY;
                        velocity_.y = 0.0f;
                        onGround_ = true;
                        onLadder_ = false;
                    } else {
                        // 上面よりも十分上にいる場合は、ハシゴを離脱して重力へ復帰
                        onLadder_ = false;
                    }
                } else {
                    // 頭上にブロックがない: ハシゴを離脱して重力へ復帰
                    onLadder_ = false;
                }
            } else {
                onLadder_ = false;
            }
        }

        // ハシゴ上でジャンプ入力があれば、ハシゴ状態を離脱してジャンプを実行
        bool jumpPressed = Input::GetInstance()->PushKey(DIK_UP) || Input::GetInstance()->PushKey(DIK_SPACE) || (state.Gamepad.wButtons & XINPUT_GAMEPAD_A);
        if (jumpPressed) {
            onLadder_ = false;
            // 小さめのジャンプを実行
            velocity_.y = kJumpVelocityGround;
        }

        // 早期リターンして通常の重力処理をスキップ
        // 注意: この後の衝突と最終反映処理は継続
        return;
    }

    if (onGround_) {
        if (moveRight || moveLeft) {
            Vector3 acceleration = {};

            float inputIntensityRight = (keyRight) ? 1.0f : std::max(0.0f, stickX);
            float inputIntensityLeft = (keyLeft) ? 1.0f : std::max(0.0f, -stickX);

            if (inputIntensityRight > 0.0f) {
                if (velocity_.x < 0.0f) {
                    // 反転時の減衰量を摩擦に応じて決定
                    float reverseDamp = onIce_ ? 0.8f : 0.3f;
                    velocity_.x *= reverseDamp;

                    if (std::fabs(velocity_.x) < 0.01f)
                        velocity_.x = 0.0f;
                }
                if (lrDirection_ != LRDirection::kRight) {
                    lrDirection_ = LRDirection::kRight;
                    turnFirstRotationY_ = worldTransform_.rotation_.y;
                    turnTimer_ = kTimeTurn;
                }
                acceleration.x += (onIce_ ? (kAcceleration * 0.6f) : kAcceleration) * inputIntensityRight;
            } else if (inputIntensityLeft > 0.0f) {
                if (velocity_.x > 0.0f) {
                    float reverseDamp = onIce_ ? 0.8f : (1.0f - groundFriction * 0.7f);
                    velocity_.x *= reverseDamp;
                    if (std::fabs(velocity_.x) < 0.01f)
                        velocity_.x = 0.0f;
                }
                if (lrDirection_ != LRDirection::kLeft) {
                    lrDirection_ = LRDirection::kLeft;
                    turnFirstRotationY_ = worldTransform_.rotation_.y;
                    turnTimer_ = kTimeTurn;
                }
                acceleration.x -= (onIce_ ? (kAcceleration * 0.6f) : kAcceleration) * inputIntensityLeft;
            }

            velocity_.x += acceleration.x;
            velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);
        } else {
            // 地上での減衰（氷上では減衰大幅に弱め）
            // 摩擦係数に応じて減衰量をスケール: 摩擦が高いほど強い減衰
            float baseAtten = kAttenuation;
            float atten = (onIce_) ? (kAttenuation * 0.15f) : (baseAtten * (1.0f + (1.0f - groundFriction)));
            velocity_.x *= (1.0f - atten);
        }

        // ジャンプ入力はライズエッジ側で処理するため、ここで直接加算は行わない

    } else {
        // 空中の横移動制御は変更なし
        if (moveRight || moveLeft) {
            float inputIntensityRight = (keyRight) ? 1.0f : std::max(0.0f, stickX);
            float inputIntensityLeft = (keyLeft) ? 1.0f : std::max(0.0f, -stickX);

            float accelX = 0.0f;
            if (inputIntensityRight > 0.0f) {
                // 反対方向への速度を少し緩和して方向転換を行う
                if (velocity_.x < 0.0f) {
                    velocity_.x *= 0.8f;
                }
                if (lrDirection_ != LRDirection::kRight) {
                    lrDirection_ = LRDirection::kRight;
                    turnFirstRotationY_ = worldTransform_.rotation_.y;
                    turnTimer_ = kTimeTurn;
                }
                accelX += kAirAcceleration * inputIntensityRight;
            } else if (inputIntensityLeft > 0.0f) {
                if (velocity_.x > 0.0f) {
                    velocity_.x *= 0.8f;
                }
                if (lrDirection_ != LRDirection::kLeft) {
                    lrDirection_ = LRDirection::kLeft;
                    turnFirstRotationY_ = worldTransform_.rotation_.y;
                    turnTimer_ = kTimeTurn;
                }
                accelX -= kAirAcceleration * inputIntensityLeft;
            }

#ifdef _DEBUG
			DebugText::GetInstance()->ConsolePrintf("AirInput onGround=%s inputR=%.3f inputL=%.3f accelX=%.3f velBefore=%.3f\n",
				onGround_ ? "true" : "false", inputIntensityRight, inputIntensityLeft, accelX, velocity_.x);
#endif

			velocity_.x += accelX;
			// 空中では地上より少し低い最大速度に制限
			velocity_.x = std::clamp(velocity_.x, -kAirLimitRunSpeed, kAirLimitRunSpeed);

#ifdef _DEBUG
			DebugText::GetInstance()->ConsolePrintf(" -> velAfter=%.3f\n", velocity_.x);
#endif
        } else {
            // 空中では減衰を弱める（空中の慣性を残す）
            velocity_.x *= (1.0f - kAttenuation * 0.2f);
        }

        // 常に重力を加える（空中）
        velocity_.y += -kGravityAcceleration;

        // 落下速度の上限を設ける
        if (velocity_.y < -kLimitFallSpeed) {
            velocity_.y = -kLimitFallSpeed;
        }
    }

    
		bool keyJumpDown = Input::GetInstance()->PushKey(DIK_UP) || Input::GetInstance()->PushKey(DIK_SPACE);
	bool keyboardRising = keyJumpDown && !prevJumpKeyPressed_;
	prevJumpKeyPressed_ = keyJumpDown;

	bool gamepadA = (state.Gamepad.wButtons & XINPUT_GAMEPAD_A) != 0;
	bool gamepadRising = gamepadA && !prevAButtonPressed_;
	prevAButtonPressed_ = gamepadA;

	if ((keyboardRising || gamepadRising) && (onGround_ || jumpCount_ < kMaxJumps)) {
#ifdef _DEBUG
		// デバッグ出力: ジャンプ発生時の情報を表示
		{
			float beforeVY = velocity_.y;
			DebugText::GetInstance()->ConsolePrintf("JumpTrigger onGround=%s jumpCount=%d beforeVY=%.3f posY=%.3f\n",
                onGround_ ? "true" : "false", jumpCount_, beforeVY, worldTransform_.translation_.y);
			if (mapChipField_) {
				// 各コーナー下のマップチップタイプを表示
				for (int i = 0; i < static_cast<int>(kNumCorners); ++i) {
					Vector3 cornerPos = CornerPosition(worldTransform_.translation_, static_cast<Player::Corner>(i));
					IndexSet idx = mapChipField_->GetMapChipIndexSetByPosition(cornerPos - Vector3{0.0f, 0.05f, 0.0f});
					MapChipType type = mapChipField_->GetMapChipTypeByIndex(idx.xIndex, idx.yIndex);
					DebugText::GetInstance()->ConsolePrintf(" corner=%d idx=(%d,%d) type=%d\n", i, idx.xIndex, idx.yIndex, static_cast<int>(type));
				}
			}
		}
#endif

		if (onGround_) {
			// 地上ジャンプは前フレームの垂直速度を消してから固定上向き速度を与える
			velocity_.y = 0.0f;
			velocity_.y = kJumpVelocityGround;
			// 地上からジャンプした瞬間は横方向の慣性を抑える
			velocity_.x *= kJumpHorizontalDamp;
			// ただし空中での上限に収める
			velocity_.x = std::clamp(velocity_.x, -kAirLimitRunSpeed, kAirLimitRunSpeed);
		} else {
			// 二段ジャンプも固定上向き速度を設定して高さを安定させる
			velocity_.y = kJumpVelocityAir;
		}
		// 急速な二段ジャンプ入力で想定外に高くなるのを防ぐため、上向き速度を上限でクランプする
	

		// play jump sound asynchronously
		if (seJumpDecisionDataHandle_ != 0u) {
			Audio::GetInstance()->PlayWave(seJumpDecisionDataHandle_, false, 1.0f);
		}
		jumpCount_++;
		onGround_ = false;

#ifdef _DEBUG
		DebugText::GetInstance()->ConsolePrintf(" -> afterVY=%.3f jumpCount=%d\n", velocity_.y, jumpCount_);
#endif
	}
}

void Player::Update() {

	if (!isAlive_) {
		return;
	}

	// Update rumble state so vibration stops after its duration
	UpdateRumble();

	if (isDying_) {
		deathDelayTimer_ -= 1.0f / 60.0f;
		if (deathDelayTimer_ <= 0.0f) {
			deathDelayTimer_ = 0.0f;
			isAlive_ = false;
			isDying_ = false;
		}
		return;
	}

	if (invincible_) {
		invincibleTimer_ -= 1.0f / 60.0f;
		if (invincibleTimer_ <= 0.0f) {
			invincible_ = false;
			invincibleTimer_ = 0.0f;
		}
	}
	// 攻撃クールタイム減算
	if (attackCooldown_ > 0.0f) {
		attackCooldown_ -= 1.0f / 60.0f;
		if (attackCooldown_ < 0.0f) attackCooldown_ = 0.0f;
	}

	// 攻撃入力バッファ減算
	if (attackInputBufferTimer_ > 0.0f) {
		attackInputBufferTimer_ -= 1.0f / 60.0f;
		if (attackInputBufferTimer_ < 0.0f) attackInputBufferTimer_ = 0.0f;
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
		BehaviorRootUpdate();
		break;
	case Behavior::kAttack:
		BehaviorAttackUpdate();
		break;
	default:
		BehaviorRootUpdate();
		break;
	}

	// 1. 移動入力
	HandleMovementInput();

	// 攻撃入力: Eキー（キーボード）またはRT（Xbox）の立ち上がり検知
	bool eTriggered = Input::GetInstance()->TriggerKey(DIK_E);
	bool rtPressed = (state.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
	bool rtRising = rtPressed && !prevRightTriggerPressed_;

	// 入力が来たらバッファに蓄える
	if (eTriggered || rtRising) {
		attackInputBufferTimer_ = kAttackInputBufferTime;
	}

	// クールタイム終了かつ未攻撃中、かつバッファが残っていれば攻撃開始
	if (attackCooldown_ <= 0.0f && !IsAttacking() && attackInputBufferTimer_ > 0.0f) {
		behaviorRequest_ = Behavior::kAttack;
		attackInputBufferTimer_ = 0.0f; // 消費
	}
	// 現在のRT状態を保存（次フレームとの比較用）
	prevRightTriggerPressed_ = rtPressed;

	// 攻撃が要求されているフレームは回避入力処理を抑制（入力競合対策）
	// NOTE: Q-trigger dodge is handled in HandleMovementInput(). Here we update dodge timers.

	// decrease dodge timer if currently dodging; end dodge when timer elapses
	if (isDodging_) {
		dodgeTimer_ -= 1.0f / 60.0f;
		if (dodgeTimer_ <= 0.0f) {
			isDodging_ = false;
			// reduce horizontal speed when dodge ends
			velocity_.x *= 0.5f;
			velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);
			// restore visual crouch if it was forced by dodge
			if (crouchForcedByDodge_) {
				float oldHalfHeight = kHeight * 0.5f * worldTransform_.scale_.y;
				worldTransform_.scale_.y = baseScaleY_;
				float newHalfHeight = kHeight * 0.5f * worldTransform_.scale_.y;
				worldTransform_.translation_.y += (newHalfHeight - oldHalfHeight);
				isCrouching_ = false;
				crouchForcedByDodge_ = false;
				UpdateAABB();
			}
		}
	}

	// dodge cooldown decrement
	if (dodgeCooldown_ > 0.0f) {
		dodgeCooldown_ -= 1.0f / 60.0f;
		if (dodgeCooldown_ < 0.0f)
			dodgeCooldown_ = 0.0f;
	}


	// 衝突情報を初期化
	CollisionMapInfo collisionInfo;
	// 移動量を加味して現在地を算定するために、現在の速度をcollisionInfoにセット
	collisionInfo.movement_ = velocity_;

	// 2. 移動量を加味して衝突判定する（軸ごとに解決してガタつきを抑制）"
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
		    std::numbers::pi_v<float> / 2.0f,       // 右向き
		    std::numbers::pi_v<float> * 3.0f / 2.0f // 左向き
		};
		float destination = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];
		worldTransform_.rotation_.y = turnFirstRotationY_ + (destination - turnFirstRotationY_) * easeT;
	} else {
		float destinationRotationYTable[] = {std::numbers::pi_v<float> / 2.0f, std::numbers::pi_v<float> * 3.0f / 2.0f};
		worldTransform_.rotation_.y = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];
	}

	// Update AABB: adjust height when crouching
	UpdateAABB();

	// 8. 行列計算
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();

	// 攻撃中は攻撃エフェクトのワールド変換も更新
	if (behavior_ == Behavior::kAttack) {
		UpdateAttackEffectTransform();
		attackWorldTransform_.matWorld_ = MakeAffineMatrix(attackWorldTransform_.scale_, attackWorldTransform_.rotation_, attackWorldTransform_.translation_);
		attackWorldTransform_.TransferMatrix();
	}
}

void Player::Draw() {

	// ここに3Dモデルインスタンスの描画処理を記述する
	if (model_) {

		model_->Draw(worldTransform_, *camera_);
	}

	// 攻撃時に攻撃エフェクトをプレイヤーの前方に描画
	if (attackModel_ && behavior_ == Behavior::kAttack) {
		attackModel_->Draw(attackWorldTransform_, *camera_, textureHandle_);
	}
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

#ifdef _DEBUG
	DebugText::GetInstance()->ConsolePrintf("mapChipCollisionCheck: originalPos=(%.3f,%.3f) movement=(%.3f,%.3f)\n",
		originalPos.x, originalPos.y, info.movement_.x, info.movement_.y);
#endif

	// --- X軸 ---
	CollisionMapInfo xInfo; // 局所的に使用
	xInfo.movement_ = {info.movement_.x, 0.0f, 0.0f};
	HandleMapCollisionLeft(xInfo);
	HandleMapCollisionRight(xInfo);
	// Xの結果を適用
	float dx = xInfo.movement_.x;
	info.isWallContact_ = xInfo.isWallContact_;
	info.wallSide_ = xInfo.wallSide_;

#ifdef _DEBUG
	DebugText::GetInstance()->ConsolePrintf(" map X result: dx=%.3f isWallContact=%s wallSide=%d\n", dx, info.isWallContact_ ? "true" : "false", static_cast<int>(info.wallSide_));
#endif

	if (info.isWallContact_) {
		if (lastWallSide_ != info.wallSide_) {

			wallJumpCount_ = 0;
			lastWallSide_ = info.wallSide_;
		}

		wallContactGraceTimer_ = kWallContactGraceTime;
	} else {
		lastWallSide_ = WallSide::kNone;
	}

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

#ifdef _DEBUG
	DebugText::GetInstance()->ConsolePrintf(" map Y result: dy=%.3f isCeiling=%s isLanding=%s\n", dy, info.isCeilingCollision_ ? "true" : "false", info.isLanding_ ? "true" : "false");
#endif

	// 一時変更を戻す
	worldTransform_.translation_ = originalPos;

	// 合成結果
	info.movement_ = {dx, dy, 0.0f};
}

// 判定結果を反映して移動
void Player::JudgmentResult(const CollisionMapInfo& info) {
	// 移動
#ifdef _DEBUG
	DebugText::GetInstance()->ConsolePrintf("JudgmentResult: beforePos=(%.3f,%.3f) movement=(%.3f,%.3f)\n",
		worldTransform_.translation_.x, worldTransform_.translation_.y, info.movement_.x, info.movement_.y);
#endif

	worldTransform_.translation_ += info.movement_;

#ifdef _DEBUG
	DebugText::GetInstance()->ConsolePrintf("JudgmentResult: afterPos=(%.3f,%.3f)\n", worldTransform_.translation_.x, worldTransform_.translation_.y);
#endif

	// マップの移動可能領域に基づいて X をクランプする（左端より外に行けないようにする）　
	if (mapChipField_) {
		Rect area = mapChipField_->GetMovableArea();
		float halfWidth = kWidth * 0.5f;
		float minX = area.left + halfWidth;
		float maxX = area.right - halfWidth;
		if (minX > maxX) {
			// マップが小さすぎる場合の保険
			minX = maxX = (area.left + area.right) * 0.5f;
		}
		if (worldTransform_.translation_.x < minX) {
#ifdef _DEBUG
			DebugText::GetInstance()->ConsolePrintf("JudgmentResult: clamped left from %.3f to %.3f\n", worldTransform_.translation_.x, minX);
#endif
			worldTransform_.translation_.x = minX;
			velocity_.x = 0.0f;
		} else if (worldTransform_.translation_.x > maxX) {
#ifdef _DEBUG
			DebugText::GetInstance()->ConsolePrintf("JudgmentResult: clamped right from %.3f to %.3f\n", worldTransform_.translation_.x, maxX);
#endif
			worldTransform_.translation_.x = maxX;
			velocity_.x = 0.0f;
		}
	}
}

// 天井に接触している場合
void Player::HitCeilingCollision(CollisionMapInfo& info) {
	if (info.isCeilingCollision_) {
		DebugText::GetInstance()->ConsolePrintf("hit ceiling\n");
		velocity_.y = 0;
	}
}

void Player::HitWallCollision(CollisionMapInfo& info) {

	if (!info.isWallContact_)
		return;

	// 空中時に壁へ押し込むような速度のみ制限
	if (!onGround_) {
		if ((info.wallSide_ == WallSide::kLeft && velocity_.x < 0.0f) || (info.wallSide_ == WallSide::kRight && velocity_.x > 0.0f)) {
			velocity_.x *= 0.2f; // 完全に0にせず、勢いを少し残す
		}
	}
}

void Player::SwitchingTheGrounding(CollisionMapInfo& info) {
	// 自キャラが接地状態
	if (onGround_) {
		// ジャンプ開始
		if (velocity_.y > 0.0f) {
			onGround_ = false;
			groundMissCount_ = 0;
#ifdef _DEBUG
			DebugText::GetInstance()->ConsolePrintf("SwitchingTheGrounding: left ground because vy=%.3f\n", velocity_.y);
#endif
		} else {
			// 改良: 底面を複数点サンプリングして接地判定を安定化
			constexpr float kGroundCheckExtra = 0.02f; // bottomから少し下をサンプリング
			constexpr int kSampleCount = 3;
			std::array<float, kSampleCount> sampleXOffsets = { -kWidth * 0.45f, 0.0f, kWidth * 0.45f };

			bool hit = false;
			// まずセンター下を必須チェック（小さな浮遊足場上で端だけ外れるのを防ぐ）
			Vector3 centerSamplePos = worldTransform_.translation_ + Vector3{ 0.0f, - (kHeight * 0.5f) - kGroundCheckExtra, 0.0f };
			IndexSet centerIdx = mapChipField_->GetMapChipIndexSetByPosition(centerSamplePos);
			MapChipType centerType = mapChipField_->GetMapChipTypeByIndex(centerIdx.xIndex, centerIdx.yIndex);
#ifdef _DEBUG
			DebugText::GetInstance()->ConsolePrintf("GroundSample center pos=(%.3f,%.3f) idx=(%d,%d) type=%d\n", centerSamplePos.x, centerSamplePos.y, centerIdx.xIndex, centerIdx.yIndex, static_cast<int>(centerType));
#endif
			if (centerType == MapChipType::kBlock || centerType == MapChipType::kIce) {
				hit = true; // 中心に足場があれば地面あり
				// 追加で左右を確認して安定化（あればより確実）"
				for (int i = 0; i < kSampleCount; ++i) {
					if (i == 1) continue; // centerは既に見た
					Vector3 samplePos = worldTransform_.translation_ + Vector3{ sampleXOffsets[i], - (kHeight * 0.5f) - kGroundCheckExtra, 0.0f };
					IndexSet idx = mapChipField_->GetMapChipIndexSetByPosition(samplePos);
					
#ifdef _DEBUG
					MapChipType type = mapChipField_->GetMapChipTypeByIndex(idx.xIndex, idx.yIndex);
					DebugText::GetInstance()->ConsolePrintf("GroundSample i=%d pos=(%.3f,%.3f) idx=(%d,%d) type=%d\n", i, samplePos.x, samplePos.y, idx.xIndex, idx.yIndex, static_cast<int>(type));
#endif
				}
			} else {
				// 中心に地面がないなら周辺もチェックして、2点以上当たっていれば地面ありとみなす
				int hits = 0;
				for (int i = 0; i < kSampleCount; ++i) {
					Vector3 samplePos = worldTransform_.translation_ + Vector3{ sampleXOffsets[i], - (kHeight * 0.5f) - kGroundCheckExtra, 0.0f };
					IndexSet idx = mapChipField_->GetMapChipIndexSetByPosition(samplePos);
					MapChipType type = mapChipField_->GetMapChipTypeByIndex(idx.xIndex, idx.yIndex);
#ifdef _DEBUG
					DebugText::GetInstance()->ConsolePrintf("GroundSample i=%d pos=(%.3f,%.3f) idx=(%d,%d) type=%d\n", i, samplePos.x, samplePos.y, idx.xIndex, idx.yIndex, static_cast<int>(type));
#endif
					if (type == MapChipType::kBlock || type == MapChipType::kIce || type == MapChipType::kSpike) {
						hits++;
					}
				}
				if (hits >= 2) {
					hit = true;
				}
			}

			if (!hit) {
				// ミス -> カウントを増やし、閾値超えたら離地扱い
				groundMissCount_++;
#ifdef _DEBUG
				DebugText::GetInstance()->ConsolePrintf("SwitchingTheGrounding: ground miss count=%d\n", groundMissCount_);
#endif
				if (groundMissCount_ >= kGroundMissThreshold) {
					onGround_ = false;
					groundMissCount_ = 0;
#ifdef _DEBUG
					DebugText::GetInstance()->ConsolePrintf("SwitchingTheGrounding: leaving ground after misses\n");
#endif
				}
			} else {
				// ヒット -> リセット
				groundMissCount_ = 0;
			}
		}

	} else {

		if (info.isLanding_) {
			// 着地状態に切り替える
			onGround_ = true;

			// 着地時にX座標を減衰（氷上では弱め）
			float atten = onIce_ ? kAttenuationLanding * 0.3f : kAttenuationLanding;
			velocity_.x *= (1.0f - atten);

			// Y座標をゼロにする
			velocity_.y = 0.0f;

			// 二段ジャンプのリセット
			jumpCount_ = 0;

			// ここで現在足元のタイルが Ice かどうかを更新
			Vector3 centerSamplePos = worldTransform_.translation_ + Vector3{ 0.0f, - (kHeight * 0.5f) - 0.02f, 0.0f };
			IndexSet centerIdx = mapChipField_->GetMapChipIndexSetByPosition(centerSamplePos);
			MapChipType centerType = mapChipField_->GetMapChipTypeByIndex(centerIdx.xIndex, centerIdx.yIndex);
			onIce_ = (centerType == MapChipType::kIce);

#ifdef _DEBUG
			DebugText::GetInstance()->ConsolePrintf("SwitchingTheGrounding: landed via isLanding_=true dy=%.3f onIce=%s\n", info.movement_.y, onIce_ ? "true" : "false");
#endif

			wallJumpCount_ = 0;
		}
	}
}

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
	if (mapChipType == MapChipType::kBlock || mapChipType == MapChipType::kIce) {
		hit = true;
	}

	// 右上点の座標
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock || mapChipType == MapChipType::kIce) {
		hit = true;
	}

	// 衝突している場合
	if (hit) {

		IndexSet indexSetNow;
		// 現在の左上点のタイルと比較して、上方向への遷移を検出
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(worldTransform_.translation_, kLeftTop));
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftTop]);
#ifdef _DEBUG
		DebugText::GetInstance()->ConsolePrintf("HandleMapCollisionUp: indexNow=(%d,%d) indexNew=(%d,%d)\n", indexSetNow.xIndex, indexSetNow.yIndex, indexSet.xIndex, indexSet.yIndex);
#endif
		if (indexSetNow.yIndex != indexSet.yIndex) {

			// めり込み先ブロックの範囲矩形
			Rects rects = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);

#ifdef _DEBUG
			DebugText::GetInstance()->ConsolePrintf(" HandleMapCollisionUp: rect top=%.3f bottom=%.3f\n", rects.top, rects.bottom);
#endif

			info.movement_.y = std::max(0.0f, rects.bottom - worldTransform_.translation_.y - (kHeight * 0.5f + kBlank));

			// 天井に当たったことを記録する
			info.isCeilingCollision_ = true;
		}
	}
}

void Player::HandleMapCollisionDown(CollisionMapInfo& info) {
	std::vector<Corner> corners = {kLeftBottom, kRightBottom};

	for (Corner corner : corners) {
		// 移動後の予測座標でチェック
		Vector3 pos = CornerPosition(worldTransform_.translation_ + info.movement_, corner);

		// 座標からインデックスを取得
		IndexSet index = mapChipField_->GetMapChipIndexSetByPosition(pos);
		MapChipType type = mapChipField_->GetMapChipTypeByIndex(index.xIndex, index.yIndex);

		if (type == MapChipType::kBlock || type == MapChipType::kIce || type == MapChipType::kSpike) {
			// ★ここがポイント：ブロックの「実際の座標」を取得する
			Rects rect = mapChipField_->GetRectByIndex(index.xIndex, index.yIndex);

			// ブロックの天面(rect.top)にプレイヤーの足元を合わせる
			// プレイヤーの足元のY座標は (translation_.y - kHeight / 2.0f)
			float footY = worldTransform_.translation_.y - (kHeight / 2.0f);

			// めり込みしている分を補正量として計算
			// 足元が rect.top より下にある場合、その差分を押し戻す
			float pushUp = rect.top - footY;

			info.movement_.y = pushUp;
			velocity_.y = 0.0f;
			info.isLanding_ = true;
			return;
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
	if (mapChipType == MapChipType::kBlock || mapChipType == MapChipType::kIce) {
		hit = true;
	}

	// 左下点の座標
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock || mapChipType == MapChipType::kIce) {
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
			float dxAllowed = (rects.right + kBlank) - (worldTransform_.translation_.x - kWidth * 0.5f);
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
	if (mapChipType == MapChipType::kBlock || mapChipType == MapChipType::kIce) {
		hit = true;
	}

	// 右下点の座標
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock || mapChipType == MapChipType::kIce) {
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
			float dxAllowed = (rects.left - kBlank) - (worldTransform_.translation_.x + kWidth * 0.5f);
			info.movement_.x = std::max(0.0f, std::min(info.movement_.x, dxAllowed));

			info.isWallContact_ = true;
			info.wallSide_ = WallSide::kRight;
		}
	}
}

void Player::UpdateWallSlide(const CollisionMapInfo& info) {

	static WallSide prevWallSide = WallSide::kNone;

	// クールダウン減算
	if (wallJumpCooldown_ > 0.0f) {
		wallJumpCooldown_ -= 1.0f / 60.0f;
		wallJumpCooldown_ = std::max(wallJumpCooldown_, 0.0f);
	}

	isWallSliding_ = false;
	if (onGround_) {
		prevWallSide = WallSide::kNone;
		return;
	}

	if (info.isWallContact_ && velocity_.y < 0.0f) {
		bool pressingTowardWall = IsPressingTowardWall(state, info.wallSide_);
		if (pressingTowardWall) {
			isWallSliding_ = true;
			velocity_.y = std::max(velocity_.y, -kWallSlideMaxFallSpeed);

			// 壁を切り替えたら即ジャンプできるようにクールダウン解除
			if (prevWallSide != info.wallSide_) {
				wallJumpCooldown_ = 0.0f;
			}
		}
	}
#ifdef _DEBUG
	ImGui::Begin("Wall Debug");
	ImGui::Text("onGround: %s", onGround_ ? "true" : "false");
	ImGui::Text("isWallContact: %s", info.isWallContact_ ? "true" : "false");
	ImGui::Text("isWallSliding: %s", isWallSliding_ ? "true" : "false");
	ImGui::Text("velocityY: %.3f", velocity_.y);
	const char* facing = (lrDirection_ == LRDirection::kRight) ? "Right" : "Left";
	ImGui::Text("playerFacing: %s", facing);
	ImGui::End();
#endif

	prevWallSide = info.wallSide_;
}

void Player::HandleWallJump(const CollisionMapInfo& info) {
	if (onGround_) {
		return;
	}

	bool canWallJump = (isWallSliding_ || info.isWallContact_);
	if (!canWallJump) {
		return;
	}

	// 入力緩和：ジャンプ押しっぱでも短時間なら再入力扱い
	static float jumpBufferTimer = 0.0f;
	// SPACEもジャンプとして扱う
	bool jumpPressed = Input::GetInstance()->PushKey(DIK_UP) || Input::GetInstance()->PushKey(DIK_SPACE) || (state.Gamepad.wButtons & XINPUT_GAMEPAD_A);
	if (jumpPressed) {
		jumpBufferTimer = 0.15f; // 0.15秒以内ならジャンプ受付
	} else {
		jumpBufferTimer -= 1.0f / 60.0f;
		jumpBufferTimer = std::max(jumpBufferTimer, 0.0f);
	}

	// 制限：壁ジャンプ回数を超えないようにする
	if (jumpBufferTimer > 0.0f && wallJumpCooldown_ <= 0.0f && wallJumpCount_ < kMaxWallJumps) {

		// 1回目と2回目で挙動を少し変える
		float horizSpeed = (wallJumpCount_ == 0) ? kWallJumpHorizontalSpeed : kWallJumpHorizontalSpeed2;
		float vertSpeed = (wallJumpCount_ == 0) ? kWallJumpVerticalSpeed : kWallJumpVerticalSpeed2;

		// 接触壁の反対方向へ跳ねる
		if (info.wallSide_ == WallSide::kLeft) {
			velocity_.x = +horizSpeed;
			lrDirection_ = LRDirection::kRight;
		} else if (info.wallSide_ == WallSide::kRight) {
			velocity_.x = -horizSpeed;
			lrDirection_ = LRDirection::kLeft;
		}

		// 上方向へ加速（高すぎないようにクリップ）
		velocity_.y = vertSpeed;
		velocity_.y = std::min(velocity_.y, vertSpeed);

		// 壁ジャンプ直後の横方向制御をやわらかくするために減衰をかける
		velocity_.x *= kWallJumpHorizontalDamp;

		isWallSliding_ = false;

		// 壁ジャンプはジャンプ回数を1にする（空中での二段ジャンプを一回許可）-
		jumpCount_ = 1;

		// プレイヤーの入力に応じて微調整を許可（操作性向上）
		if (Input::GetInstance()->PushKey(DIK_LEFT) || Input::GetInstance()->PushKey(DIK_A)) {
			velocity_.x -= 0.15f;
		}
		if (Input::GetInstance()->PushKey(DIK_RIGHT) || Input::GetInstance()->PushKey(DIK_D)) {
			velocity_.x += 0.15f;
		}

		// 旋回演出
		turnFirstRotationY_ = worldTransform_.rotation_.y;
		turnTimer_ = kTimeTurn;

		// 連続発動防止
		wallJumpCooldown_ = kWallJumpCooldownTime;
		jumpBufferTimer = 0.0f; // 消費

		// カウントを増やす
		wallJumpCount_ = std::min(wallJumpCount_ + 1, kMaxWallJumps);
	}
}

void Player::OnCollision(Enemy* enemy) {

	if (behavior_ == Behavior::kAttack) {
		return;
	}


	if (isDying_ || !isAlive_)
		return;


	if (invincible_)
		return;

	(void)enemy;
	
	hp_ = std::max(0, hp_ - 1);

	DebugText::GetInstance()->ConsolePrintf("Player damaged. HP=%d\n", hp_);

	// Play damage sound (async)
	if (seDamageSoundHandle_ != 0u) {
		Audio::GetInstance()->PlayWave(seDamageSoundHandle_, false, 1.0f);
	}
	invincible_ = true;
	invincibleTimer_ = kInvincibleDuration;

	
	// Start a milder, shorter rumble on damage (intensity 0.4, duration 500 ms)
	// change duration to 1000 ms (1 second)
	// milder and shorter rumble: intensity 0.25, duration 250 ms
	StartRumble(0.25f, 0.25f, 250);
	if (cameraController_)
		cameraController_->StartShake(0.8f, 0.12f);

	
	if (hp_ <= 0) {
		isDying_ = true;
		deathDelayTimer_ = kDeathDelay;

		
		velocity_ = {0.0f, 0.0f, 0.0f};
	}
}

void Player::UpdateAABB() {
	// プレイヤーと同等サイズの簡易AABB（必要なら調整）"

	static constexpr float kDepth = 0.8f * 2.0f;

	Vector3 center = worldTransform_.translation_;
	// adjust height based on crouch: use multiplier
	float heightMultiplier = isCrouching_ ? kCrouchHeightMultiplier : 1.0f;
	float effectiveHeight = kHeight * heightMultiplier;
	Vector3 half = {kWidth * 0.5f, effectiveHeight * 0.5f, kDepth * 0.5f};

	// 回転は無視して軸整列AABBを更新（必要ならメッシュ頂点から算出実装へ拡張）
	aabb_.min = {center.x - half.x, center.y - half.y, center.z - half.z};
	aabb_.max = {center.x + half.x, center.y + half.y, center.z + half.z};
}

void Player::EmergencyAvoidance() {
	// Dodge via Q is handled once in HandleMovementInput using TriggerKey(DIK_Q).
	// Keep this function empty to avoid duplicate or continuous triggers.
}

void Player::BehaviorRootInitialize() {}

void Player::BehaviorAttackInitialize() {
	// カウンターの初期化
	attackParameter_ = 0;
	// 攻撃クールタイム開始
	attackCooldown_ = kAttackCooldownTime;

	// 攻撃開始時点のエフェクトの初期位置更新
	UpdateAttackEffectTransform();

	// play attack sound asynchronously
	if (seAttackSoundHandle_ != 0u) {
		Audio::GetInstance()->PlayWave(seAttackSoundHandle_, false, 1.0f);
	}
}

void Player::BehaviorRootUpdate() {}

void Player::BehaviorAttackUpdate() {

	// 攻撃動作時間経過
	attackParameter_++;

	if (attackParameter_ > kAttackDuration) {
		behaviorRequest_ = Behavior::kRoot;
	}

	// ダッシュ切り実装: 攻撃開始から最初の数フレームだけ前方へ短距離ダッシュ
	float dir = (lrDirection_ == LRDirection::kRight) ? 1.0f : -1.0f;

	if (attackParameter_ <= kAttackDashFrames) {
		// 攻撃開始フレームは強制的にダッシュ速度を与える
		velocity_.x = dir * kAttackDashSpeed;
	} else {
		// ダッシュ終了後は急速に減衰させて停止に持っていく
		velocity_.x *= 0.5f;
		// 安全にクランプ
		velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);
	}

	// 攻撃エフェクト追従
	UpdateAttackEffectTransform();

	// 縦方向は通常の物理挙動に任せる。
}

void Player::UpdateAttackEffectTransform() {
	// プレイヤーの進行方向前にエフェクトを配置
	float dirSign = (lrDirection_ == LRDirection::kRight) ? 1.0f : -1.0f;

	// 前方オフセット（プレイヤー幅の半分 + 攻撃幅の半分より少し前に）	
	const float forwardOffset = (kWidth * 0.5f) + (kAttackWidth * 0.5f) * 0.8f;

	attackWorldTransform_.scale_ = {worldTransform_.scale_.x * kAttackEffectScale,
						 worldTransform_.scale_.y * kAttackEffectScale,
						 worldTransform_.scale_.z * kAttackEffectScale};
	attackWorldTransform_.rotation_ = worldTransform_.rotation_;
	attackWorldTransform_.translation_ = worldTransform_.translation_;
	attackWorldTransform_.translation_.x += dirSign * forwardOffset;
	// カメラに少し近づけるバイアス（Z軸）
	attackWorldTransform_.translation_.z += kAttackEffectZBias;
}



// 新しい関数: プレイヤーの攻撃用AABBを返す
AABB Player::GetAttackAABB() const {

	Vector3 center = worldTransform_.translation_;

	float dir = (lrDirection_ == LRDirection::kRight) ? 1.0f : -1.0f;

	float halfAttackWidth = kAttackWidth * 0.5f;
	float halfAttackHeight = kAttackHeight * 0.5f;

	Vector3 attackCenter = center;
	attackCenter.x += dir * (kWidth * 0.5f + halfAttackWidth);

	AABB hitbox;
	hitbox.min = {attackCenter.x - halfAttackWidth, attackCenter.y - halfAttackHeight, attackCenter.z - 1.0f};
	hitbox.max = {attackCenter.x + halfAttackWidth, attackCenter.y + halfAttackHeight, attackCenter.z + 1.0f};
	return hitbox;
	}

void Player::ConsumeKey() {
    keyCount_ += 1;
    DebugText::GetInstance()->ConsolePrintf("Player: consumed key. total keys=%d\n", keyCount_);
}

void Player::ApplyInvincibility(float duration) {
    invincible_ = true;
    invincibleTimer_ = duration;
}

void Player::SuppressNextJump() {
	// mark previous jump keys/buttons as pressed so next frame rising edge isn't detected
	prevJumpKeyPressed_ = true;
	prevAButtonPressed_ = true;
}
