#include "Key.h"

#include "KamataEngine.h"
#include "MathUtl.h"
#include "Player.h"

#include <algorithm>
#include <cmath>

using namespace KamataEngine;

void Key::Initialize() {
    frame_ = 0;

    
    model_ = KamataEngine::Model::CreateFromOBJ("Key", true);
    ownsModel_ = (model_ != nullptr);

    worldTransform_.Initialize();
    worldTransform_.translation_ = position_;
    worldTransform_.translation_.z = 0.0f;
    worldTransform_.rotation_ = {0, 0, 0};
    worldTransform_.scale_ = initialScale_;

    worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
    worldTransform_.TransferMatrix();
}

Key::~Key() {
    if (ownsModel_ && model_) {
        delete model_;
        model_ = nullptr;
    }
}

void Key::OnPicked(Player* player) {
    if (state_ != State::kIdle) return;

  
    if (player) {
        Vector3 p = player->GetPosition();
        float dx = p.x - position_.x;
        float dy = p.y - position_.y;
        float dist = std::sqrt(dx * dx + dy * dy);
        if (dist > kRotateSkipDistance) {
            state_ = State::kAttracting;
            targetPlayer_ = player;
            stateTimer_ = 0.0f;
            return;
        }
    }

    state_ = State::kRotating;
    stateTimer_ = 0.0f;
    targetPlayer_ = player;
}

bool Key::IsPicked() const { return state_ != State::kIdle; }
bool Key::IsCollected() const { return collected_; }

void Key::Update(float delta) {
    const float frameTime = 0.15f;
    static float acc = 0.0f;
    acc += delta;
    if (acc >= frameTime) {
        acc = 0.0f;
        frame_ = (frame_ + 1) % 4;
    }

   
    switch (state_) {
    case State::kIdle:
      
        break;
    case State::kRotating: {
        stateTimer_ += delta;
     
        worldTransform_.rotation_.z += kRotateSpeed * delta;
       
        if (targetPlayer_) {
            Vector3 p = targetPlayer_->GetPosition();
            float dx = p.x - position_.x;
            float dy = p.y - position_.y;
            float dist = std::sqrt(dx * dx + dy * dy);
            if (dist > kRotateSkipDistance) {
                state_ = State::kAttracting;
                stateTimer_ = 0.0f;
                break;
            }
        }
        if (stateTimer_ >= kRotateDuration) {
            state_ = State::kAttracting;
          
            stateTimer_ = 0.0f;
        }
        break;
    }
    case State::kAttracting: {
        if (!targetPlayer_) {
            state_ = State::kCollected;
            collected_ = true;
            break;
        }
       
        Vector3 playerPos = targetPlayer_->GetPosition();
      
        playerPos.z = 0.0f;
        Vector3 dir = {playerPos.x - position_.x, playerPos.y - position_.y, 0.0f};
        float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (dist > 0.0001f) {
            dir.x /= dist; dir.y /= dist;
           
            float multiplier = 1.0f;
            if (dist > kNearDistanceThreshold) {
                multiplier += (dist - kNearDistanceThreshold) * kDistanceSpeedFactor;
                multiplier = (std::min)(multiplier, kMaxAttractMultiplier);
            }
            float move = kAttractSpeed * multiplier * delta;
            if (move > dist) move = dist;
            position_.x += dir.x * move;
            position_.y += dir.y * move;
        }

    
        float scaleFactor = 1.0f;
     
        float startShrinkDist = 2.0f;
        float t = 1.0f - std::clamp(dist / startShrinkDist, 0.0f, 1.0f);
        scaleFactor = 1.0f - 0.9f * t; 
        worldTransform_.scale_ = {initialScale_.x * scaleFactor, initialScale_.y * scaleFactor, initialScale_.z * scaleFactor};

      
        worldTransform_.rotation_.z += kRotateSpeed * 2.0f * delta;

        if (dist <= kCollectDistance) {
            state_ = State::kCollected;
            collected_ = true;
        }
        break;
    }
    case State::kCollected:
     
        break;
    }

    worldTransform_.translation_ = position_;
    worldTransform_.translation_.z = 0.0f;

    worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
    worldTransform_.TransferMatrix();
}

void Key::Draw(KamataEngine::Camera* camera) {
    if (!model_) return;
    if (!camera) return;

  
    if (state_ == State::kCollected && collected_) return;

    worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
    worldTransform_.TransferMatrix();

    model_->Draw(worldTransform_, *camera);
}

AABB Key::GetAABB() const {
    static constexpr float kWidth = 0.6f * 2.0f;
    static constexpr float kHeight = 0.6f * 2.0f;
    static constexpr float kDepth = 0.6f * 2.0f;

    Vector3 center = position_;
    Vector3 half = {kWidth * 0.5f, kHeight * 0.5f, kDepth * 0.5f};

    AABB aabb;
    aabb.min = {center.x - half.x, center.y - half.y, center.z - half.z};
    aabb.max = {center.x + half.x, center.y + half.y, center.z + half.z};
    return aabb;
}

void Key::PlayGetSound() {
    KamataEngine::Audio::GetInstance()->PlayWave(soundDataHandle, false);
}
