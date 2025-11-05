#include "CameraController.h"
#include "MathUtl.h"
#include "Player.h"

using namespace KamataEngine;

CameraController::CameraController() {}
CameraController::~CameraController() {}

void CameraController::Initialize(KamataEngine::Camera* camera) {
    camera_ = camera;
}

void CameraController::Update() {
    if (!camera_ || !target_) { return; }

    const WorldTransform& targetWorldTransform = target_->GetWorldTransform();
    camera_->translation_ = targetWorldTransform.translation_ + targetOffset_;
    camera_->UpdateMatrix();
    camera_->TransferMatrix();
}

void CameraController::Draw() {}

void CameraController::Reset() {
    if (!camera_ || !target_) { return; }

    const WorldTransform& targetWorldTransform = target_->GetWorldTransform();
    camera_->translation_ = targetWorldTransform.translation_ + targetOffset_;
    camera_->UpdateMatrix();
    camera_->TransferMatrix();
}