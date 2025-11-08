#include "CameraController.h"
#include "MathUtl.h"
#include "Player.h"

#include <algorithm>

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

    camera_->translation_.x = std::clamp(camera_->translation_.x, movableArea.left + Margin.left, movableArea.right - Margin.right);
	camera_->translation_.y = std::clamp(camera_->translation_.y, movableArea.bottom + Margin.bottom, movableArea.top - Margin.top);

    camera_->translation_.x = std::clamp(camera_->translation_.x, movableArea.left, movableArea.right);
	camera_->translation_.y = std::clamp(camera_->translation_.y, movableArea.bottom, movableArea.top);

    camera_->UpdateMatrix();
  
}

void CameraController::Draw() {}

void CameraController::Reset() {
    if (!camera_ || !target_) { return; }

    const WorldTransform& targetWorldTransform = target_->GetWorldTransform();
    camera_->translation_ = targetWorldTransform.translation_ + targetOffset_;
}