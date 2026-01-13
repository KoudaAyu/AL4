#include "Enemy.h"
#include "Wall.h"
#include <limits>
#include <cfloat>
using namespace KamataEngine;

Enemy::Enemy() {}

Enemy::~Enemy() {}

void Enemy::Initialize(KamataEngine::Camera* camera, KamataEngine::Vector3 pos) { 
	camera_ = camera;
	model_ = Model::Create();
	worldTransform_.Initialize();
	worldTransform_.translation_ = pos;
}

void Enemy::Update(const std::list<Wall*>& walls) {

	#pragma region 一番近くのWallに移動する処理

	if (!walls.empty())
	{
		Wall* nearestWall = nullptr;

		float nearestDistance = FLT_MAX;

		// 一番近いWallを探す
		for (Wall* wall : walls)
		{
			if (!wall)
			{
				continue;
			}
			Vector3 toWall = wall->GetPosition() - worldTransform_.translation_;
			float distance = toWall.Length();
			if (distance < nearestDistance) {
				nearestDistance = distance;
				nearestWall = wall;
			}
		}

		if (nearestWall)
		{
			Vector3 direction = nearestWall->GetPosition() - worldTransform_.translation_;
			float dirLen = Length(direction);
			
			if (dirLen > 0.0f) {
				direction = Normalize(direction);

				float speed = 0.1f;                               // 移動速度（適宜調整）
				worldTransform_.translation_ += direction * speed; // Vector3の演算
			}
		}
	}

	#pragma endregion 一番近くのWallに移動する処理

	UpdateAABB();

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();
}

void Enemy::Draw() { model_->Draw(worldTransform_, *camera_); }

void Enemy::UpdateAABB()
{
	aabb_.min.x = worldTransform_.translation_.x - worldTransform_.scale_.x;
	aabb_.min.y = worldTransform_.translation_.y - worldTransform_.scale_.y;
	aabb_.min.z = worldTransform_.translation_.z - worldTransform_.scale_.z;

	aabb_.max.x = worldTransform_.translation_.x + worldTransform_.scale_.x;
	aabb_.max.y = worldTransform_.translation_.y + worldTransform_.scale_.y;
	aabb_.max.z = worldTransform_.translation_.z + worldTransform_.scale_.z;
}

const AABB& Enemy::GetAABB() const { return aabb_; }

void Enemy::HandleCollision() {
	
}
