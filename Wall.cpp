#include "Wall.h"


using namespace KamataEngine;

Wall::Wall() {}

Wall::~Wall() {}

void Wall::Initialize(KamataEngine::Camera* camera, const Vector3& pos) {
	
	camera_ = camera;

	model_ = Model::CreateFromOBJ("TD_Block", true);
	worldTransform_.Initialize();
	worldTransform_.translation_ = pos;
}

void Wall::Update() {
	UpdateAABB();

	worldTransform_.matWorld_ = 
		MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();
}

void Wall::Draw() { if (model_) model_->Draw(worldTransform_, *camera_); }

void Wall::UpdateAABB() {
	aabb_.min.x = worldTransform_.translation_.x - worldTransform_.scale_.x;
	aabb_.min.y = worldTransform_.translation_.y - worldTransform_.scale_.y;
	aabb_.min.z = worldTransform_.translation_.z - worldTransform_.scale_.z;

	aabb_.max.x = worldTransform_.translation_.x + worldTransform_.scale_.x;
	aabb_.max.y = worldTransform_.translation_.y + worldTransform_.scale_.y;
	aabb_.max.z = worldTransform_.translation_.z + worldTransform_.scale_.z;
}

const AABB& Wall::GetAABB() const {
	return aabb_;
}

const KamataEngine::Vector3& Wall::GetPosition() const { return worldTransform_.translation_; }

void Wall::SetRotation(const KamataEngine::Vector3& rot) { worldTransform_.rotation_ = rot; }

const KamataEngine::Vector3& Wall::GetRotation() const { return worldTransform_.rotation_; }

bool Wall::AccumulateContactFrame() {
	++contactFrames_;
	if (contactFrames_ >= kRequiredContactFrames_) {
		contactFrames_ = 0;
		--hp_;
		if (hp_ <= 0) {
			return true;
		}
	}
	return false;
}

void Wall::DecayContactFrames() {
	// 被触続していないときに徐々に接触フレームを減少させる
	if (contactFrames_ > 0) {
		contactFrames_ -= 1; // 1フレーム分だけ減らす。必要なら緩やかに変更。
		if (contactFrames_ < 0) contactFrames_ = 0;
	}
}
