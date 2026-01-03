#include "CameraController.h"
#include "MathUtl.h"
#include "Player.h"

#include <algorithm>
#include <random>
#include <cmath>

using namespace KamataEngine;

CameraController::CameraController() {}
CameraController::~CameraController() {}

void CameraController::Initialize(KamataEngine::Camera* camera) {
    camera_ = camera;
}

void CameraController::Update() {
    if (!camera_ || !target_) { return; }

    const WorldTransform& targetWorldTransform = target_->GetWorldTransform();
	targetVelocity_ = target_->GetVelocity();
    targetPosition_ = targetWorldTransform.translation_ + targetOffset_ + targetVelocity_ * kVelocityBias;
    
    camera_->translation_ = Lerp(camera_->translation_, targetPosition_, kInterpolationRate);

    // Compute world-space half extents of the camera view at the object's plane (z ~= 0)
    float camDistance = std::fabs(camera_->translation_.z); // distance from camera to world plane (z=0)
    float halfHeight = std::tan(camera_->fovAngleY * 0.5f) * camDistance;
    float halfWidth = halfHeight * camera_->aspectRatio;

    // If the map is smaller than the viewport, center the camera on the map instead of clamping
    float mapWidth = movableArea.right - movableArea.left;
    float mapHeight = movableArea.top - movableArea.bottom;

    if (mapWidth <= 2.0f * halfWidth) {
        // center X
        camera_->translation_.x = (movableArea.left + movableArea.right) * 0.5f;
    } else {
        camera_->translation_.x = std::clamp(camera_->translation_.x, movableArea.left + halfWidth, movableArea.right - halfWidth);
    }

    if (mapHeight <= 2.0f * halfHeight) {
        // center Y
        camera_->translation_.y = (movableArea.top + movableArea.bottom) * 0.5f;
    } else {
        camera_->translation_.y = std::clamp(camera_->translation_.y, movableArea.bottom + halfHeight, movableArea.top - halfHeight);
    }

    // カメラシェイクの適用
    if (isShaking_ && shakeRemaining_ > 0.0f) {
        // 減衰率（時間とともに0になる)
        float progress = 1.0f - (shakeRemaining_ / shakeDuration_);
        float decay = 1.0f - progress; // 直線的に減衰

        // ランダムオフセットを生成
        static thread_local std::mt19937 rng{std::random_device{}()};
        std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

        float ox = dist(rng) * shakeAmplitude_ * decay;
        float oy = dist(rng) * shakeAmplitude_ * decay;
        float oz = dist(rng) * shakeAmplitude_ * decay * 0.2f; // Zは小さめ

        camera_->translation_.x += ox;
        camera_->translation_.y += oy;
        camera_->translation_.z += oz;

        // 残り時間を減らす（60FPS想定の簡易更新)
        shakeRemaining_ -= 1.0f / 60.0f;
        if (shakeRemaining_ <= 0.0f) {
            isShaking_ = false;
            shakeRemaining_ = 0.0f;
            // Z軸のズレが残らないようリセット（X,Yは追従で上書きされる)
            camera_->translation_.z = targetOffset_.z; 
        }
    }

    camera_->UpdateMatrix();
  
}

void CameraController::Draw() {}

void CameraController::Reset() {
    if (!camera_ || !target_) { return; }

    const WorldTransform& targetWorldTransform = target_->GetWorldTransform();
    camera_->translation_ = targetWorldTransform.translation_ + targetOffset_;

    // ensure initial position respects movable area and viewport size
    float camDistance = std::fabs(camera_->translation_.z);
    float halfHeight = std::tan(camera_->fovAngleY * 0.5f) * camDistance;
    float halfWidth = halfHeight * camera_->aspectRatio;

    float mapWidth = movableArea.right - movableArea.left;
    float mapHeight = movableArea.top - movableArea.bottom;

    if (mapWidth <= 2.0f * halfWidth) {
        camera_->translation_.x = (movableArea.left + movableArea.right) * 0.5f;
    } else {
        camera_->translation_.x = std::clamp(camera_->translation_.x, movableArea.left + halfWidth, movableArea.right - halfWidth);
    }
    if (mapHeight <= 2.0f * halfHeight) {
        camera_->translation_.y = (movableArea.top + movableArea.bottom) * 0.5f;
    } else {
        camera_->translation_.y = std::clamp(camera_->translation_.y, movableArea.bottom + halfHeight, movableArea.top - halfHeight);
    }
}

void CameraController::StartShake(float amplitude, float duration) {
    if (duration <= 0.0f || amplitude <= 0.0f) { return; }
    shakeAmplitude_ = amplitude;
    shakeDuration_ = duration;
    shakeRemaining_ = duration;
    isShaking_ = true;
}