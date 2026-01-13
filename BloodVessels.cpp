#include "BloodVessels.h"


using namespace KamataEngine;

BloodVessels::BloodVessels() {}

BloodVessels::~BloodVessels() {}

void BloodVessels::Initialize(KamataEngine::Camera* camera, const Vector3& pos) {
	
	camera_ = camera;

	model_ = Model::CreateFromOBJ("TD_Block", true);
	worldTransform_.Initialize();
	worldTransform_.translation_ = pos;
}

void BloodVessels::Update() {
	UpdateAABB();

	worldTransform_.matWorld_ = 
		MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();
}

void BloodVessels::Draw() { model_->Draw(worldTransform_, *camera_); }

void BloodVessels::UpdateAABB() {
	aabb_.min.x = worldTransform_.translation_.x - worldTransform_.scale_.x;
	aabb_.min.y = worldTransform_.translation_.y - worldTransform_.scale_.y;
	aabb_.min.z = worldTransform_.translation_.z - worldTransform_.scale_.z;

	aabb_.max.x = worldTransform_.translation_.x + worldTransform_.scale_.x;
	aabb_.max.y = worldTransform_.translation_.y + worldTransform_.scale_.y;
	aabb_.max.z = worldTransform_.translation_.z + worldTransform_.scale_.z;
}

const AABB& BloodVessels::GetAABB() const {
	return aabb_;
}
