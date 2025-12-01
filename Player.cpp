#include "Player.h"

#include "Enemy.h"
#include "MapChipField.h"
#include "CameraController.h"

#include <Windows.h>
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


static float GetHorizontalInputIntensity(float stickX, bool keyRight, bool keyLeft) {

    if (keyRight) return 1.0f;
    if (keyLeft) return 1.0f;
   
    return std::clamp(std::fabs(stickX), 0.0f, 1.0f);
}


static bool IsPressingTowardWall(const XINPUT_STATE& state, WallSide side) {
    float stickX = NormalizeLeftStickX(state.Gamepad.sThumbLX);
    switch (side) {
    case WallSide::kLeft:
        return Input::GetInstance()->PushKey(DIK_LEFT) || Input::GetInstance()->PushKey(DIK_A) || (stickX < 0.0f);
    case WallSide::kRight:
        return Input::GetInstance()->PushKey(DIK_RIGHT) || Input::GetInstance()->PushKey(DIK_D) || (stickX > 0.0f);
    default:
        return false;
    }
}

} // anonymous namespace

Player::Player() {}

Player::~Player() {

    if (ownsModel_ && model_) {
        delete model_;
        model_ = nullptr;
    }
}

// Updated signature: no Model* parameter
void Player::Initialize(Camera* camera, const Vector3& position) {

    textureHandle_ = TextureManager::Load("attack_effect/attack_effect.png");

    // Always create player's model from OBJ
    model_ = Model::CreateFromOBJ("Player", true);
    ownsModel_ = true;

    assert(model_);

    attackModel_ = Model::CreateFromOBJ("attack_effect", true);

    camera_ = camera;

    // ワールド変換の初期化
    worldTransform_.Initialize();
    worldTransform_.translation_ = position;
    worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;

    UpdateAABB();
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

    
    float stickX = NormalizeLeftStickX(state.Gamepad.sThumbLX);

    bool keyRight = Input::GetInstance()->PushKey(DIK_RIGHT) || Input::GetInstance()->PushKey(DIK_D);
    bool keyLeft = Input::GetInstance()->PushKey(DIK_LEFT) || Input::GetInstance()->PushKey(DIK_A);

    bool moveRight = keyRight || (stickX > 0.0f);
    bool moveLeft = keyLeft || (stickX < 0.0f);

    if (onGround_) {
        if (moveRight || moveLeft) {
            Vector3 acceleration = {};

            
            float inputIntensityRight = (keyRight) ? 1.0f : std::max(0.0f, stickX);
            float inputIntensityLeft = (keyLeft) ? 1.0f : std::max(0.0f, -stickX);

            if (inputIntensityRight > 0.0f) {
                if (velocity_.x < 0.0f) {
                    velocity_.x += (1.0f - kAttenuation);
                }
                if (lrDirection_ != LRDirection::kRight) {
                    lrDirection_ = LRDirection::kRight;
                    turnFirstRotationY_ = worldTransform_.rotation_.y;
                    turnTimer_ = kTimeTurn;
                }
                acceleration.x += kAcceleration * inputIntensityRight;
            } else if (inputIntensityLeft > 0.0f) {
                if (velocity_.x > 0.0f) {
                    velocity_.x *= (1.0f - kAttenuation);
                }
                if (lrDirection_ != LRDirection::kLeft) {
                    lrDirection_ = LRDirection::kLeft;
                    turnFirstRotationY_ = worldTransform_.rotation_.y;
                    turnTimer_ = kTimeTurn;
                }
                acceleration.x -= kAcceleration * inputIntensityLeft;
            }

            velocity_.x += acceleration.x;
            velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);
        } else {
            // 地上での減衰
            velocity_.x *= (1.0f - kAttenuation);
        }

        // ジャンプ入力: キーボードの上キーまたはWキーまたはXboxコントローラのAボタン
        if (Input::GetInstance()->PushKey(DIK_UP) || Input::GetInstance()->PushKey(DIK_W) || (state.Gamepad.wButtons & XINPUT_GAMEPAD_A)) {
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

   
    bool spaceTriggered = Input::GetInstance()->TriggerKey(DIK_SPACE);
    bool rtPressed = (state.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
   
    if (spaceTriggered || (rtPressed && !prevRightTriggerPressed_)) {
        behaviorRequest_ = Behavior::kAttack;
    }
    // 現在のRT状態を保存（次フレームのため）
    prevRightTriggerPressed_ = rtPressed;

   EmergencyAvoidance();
   

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
    if (model_) {
   
        model_->Draw(worldTransform_, *camera_);
    }

    if (attackModel_ && behavior_ == Behavior::kAttack) {
    
        attackModel_->Draw(worldTransform_, *camera_, textureHandle_);
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

    // --- X軸 ---
    CollisionMapInfo xInfo; // 局所的に使用
    xInfo.movement_ = {info.movement_.x, 0.0f, 0.0f};
    HandleMapCollisionLeft(xInfo);
    HandleMapCollisionRight(xInfo);
    // Xの結果を適用
    float dx = xInfo.movement_.x;
    info.isWallContact_ = xInfo.isWallContact_;
    info.wallSide_ = xInfo.wallSide_;

    
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
    if (!isDodging_) {
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

            // Reset wall-jump count when landing
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

    // decrement grace timer
    if (wallContactGraceTimer_ > 0.0f) {
        wallContactGraceTimer_ -= 1.0f / 60.0f;
        if (wallContactGraceTimer_ < 0.0f) wallContactGraceTimer_ = 0.0f;
    }

    isWallSliding_ = false;
    if (onGround_) {
        return;
    }

    if (info.isWallContact_ && velocity_.y < 0.0f) {
        bool pressingTowardWall = IsPressingTowardWall(state, info.wallSide_);
        if (pressingTowardWall) {
            isWallSliding_ = true; 
            // 落下速度を制限
            velocity_.y = std::max(velocity_.y, -kWallSlideMaxFallSpeed);
        }
    }
}

void Player::HandleWallJump(const CollisionMapInfo& info) {
  

    bool canWallJump = (isWallSliding_ || info.isWallContact_ || wallContactGraceTimer_ > 0.0f);
    if (!canWallJump) {
        return;
    }

    // 壁ジャンプ入力: キーボードの上キーまたはWキーまたはXboxコントローラのAボタン
    bool jumpPressed = Input::GetInstance()->PushKey(DIK_UP) || Input::GetInstance()->PushKey(DIK_W) || (state.Gamepad.wButtons & XINPUT_GAMEPAD_A);
    if (jumpPressed && wallJumpCooldown_ <= 0.0f) {
      
        if (wallJumpCount_ >= kMaxWallJumps) {
            return;
        }

      
        float horizSpeed = (wallJumpCount_ == 0) ? kWallJumpHorizontalSpeed : kWallJumpHorizontalSpeed2;
        float vertSpeed = (wallJumpCount_ == 0) ? kWallJumpVerticalSpeed : kWallJumpVerticalSpeed2;

        // 反対方向へ跳ねる
        if (info.wallSide_ == WallSide::kLeft) {
            velocity_.x = +horizSpeed;
            lrDirection_ = LRDirection::kRight;
        } else if (info.wallSide_ == WallSide::kRight) {
            velocity_.x = -horizSpeed;
            lrDirection_ = LRDirection::kLeft;
        } else {
           
            float dir = (lrDirection_ == LRDirection::kRight) ? 1.0f : -1.0f;
            velocity_.x = dir * horizSpeed;
        }

      
        velocity_.x *= kWallJumpHorizontalDamp;

       
        velocity_.y = vertSpeed;
        isWallSliding_ = false;

        // increment wall jump count
        wallJumpCount_++;

        // stop grace timer after jump
        wallContactGraceTimer_ = 0.0f;

        // 旋回演出
        turnFirstRotationY_ = worldTransform_.rotation_.y;
        turnTimer_ = kTimeTurn;

        // 連続発動防止
        wallJumpCooldown_ = kWallJumpCooldownTime;
    }
}

void Player::OnCollision(Enemy* enemy) {

    if (behavior_ == Behavior::kAttack) {
        return;
    }

    (void)enemy;
    isAlive_ = false;

    if (cameraController_) {
        // 揺れ幅: 2.0f, 継続: 0.5秒（必要なら調整）
        cameraController_->StartShake(2.0f, 0.5f);
    } else {
        DebugText::GetInstance()->ConsolePrintf("cameraController_ is null\n");
    }
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

void Player::EmergencyAvoidance() {
	bool dodgePressed = Input::GetInstance()->TriggerKey(DIK_E);
	if (dodgePressed && !isDodging_ && dodgeCooldown_ <= 0.0f && behavior_ != Behavior::kAttack) {

		float dir = (lrDirection_ == LRDirection::kRight) ? 1.0f : -1.0f;
		velocity_.x = dir * kDodgeSpeed;
		isDodging_ = true;
		dodgeTimer_ = kDodgeDuration;
		dodgeCooldown_ = kDodgeCooldownTime;
		if (cameraController_)
			cameraController_->StartShake(0.5f, 0.12f);
	}

	if (isDodging_) {
		dodgeTimer_ -= 1.0f / 60.0f;
		if (dodgeTimer_ <= 0.0f) {
			isDodging_ = false;

			velocity_.x *= 0.5f;

			velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);
		}
	}
	if (dodgeCooldown_ > 0.0f) {
		dodgeCooldown_ -= 1.0f / 60.0f;
		if (dodgeCooldown_ < 0.0f)
			dodgeCooldown_ = 0.0f;
	}
}

void Player::BehaviorRootInitialize() {}

void Player::BehaviorAttackInitialize() {
//カウンターの初期化
    attackParameter_ = 0;
}

void Player::BehaviorRootUpdate() {}

void Player::BehaviorAttackUpdate() {

    // 攻撃動作時間経過
    attackParameter_++;

    if (attackParameter_ > kAttackDuration) {
        behaviorRequest_ = Behavior::kRoot;
    }

  
    velocity_.x = 0.0f;

    // 縦方向は通常の物理挙動に任せる。
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
