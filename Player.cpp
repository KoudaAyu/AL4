#include"Player.h"


using namespace KamataEngine;

Player::Player() {}

Player::~Player() {}

void Player::Initialize(KamataEngine::Camera* camera, const Vector3& pos) {

	model_ = Model::Create();

	camera_ = camera;

	worldTransform_.Initialize();
	worldTransform_.translation_ = pos;
	//必要ならModelの初期角度

}

void Player::Update() { 

	worldTransform_.translation_ += velocity;

	if (Input::GetInstance()->PushKey(DIK_A)
		||Input::GetInstance()->PushKey(DIK_D)
		||Input::GetInstance()->PushKey(DIK_W)
		||Input::GetInstance()->PushKey(DIK_S))
	{
		// 加速度の設定
		Vector3 acceleration = {};

		if (Input::GetInstance()->PushKey(DIK_A)) {
			acceleration.x = -kAcceleration;
		}

		if (Input::GetInstance()->PushKey(DIK_D)) {
			acceleration.x = kAcceleration;
		}

		if (Input::GetInstance()->PushKey(DIK_W)) {
			acceleration.y = kAcceleration;
		}

		if (Input::GetInstance()->PushKey(DIK_S)) {
			acceleration.y = -kAcceleration;
		}

		velocity = acceleration;
	}
	else {
		velocity = {};
	}

	UpdateAABB();

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();
}

void Player::Draw() { model_->Draw(worldTransform_, *camera_); }

void Player::UpdateAABB() {
	
	aabb_.min.x = worldTransform_.translation_.x - worldTransform_.scale_.x;
	aabb_.min.y = worldTransform_.translation_.y - worldTransform_.scale_.y;
	aabb_.min.z = worldTransform_.translation_.z - worldTransform_.scale_.z;

	aabb_.max.x = worldTransform_.translation_.x + worldTransform_.scale_.x;
	aabb_.max.y = worldTransform_.translation_.y + worldTransform_.scale_.y;
	aabb_.max.z = worldTransform_.translation_.z + worldTransform_.scale_.z;
}

const AABB& Player::GetAABB() const {
	return aabb_;
}

void Player::HandleCollision() {
	// 簡易処理: 衝突時は速度をゼロにして位置を戻す（ここでは速度のみリセット）
	velocity = {};
}
