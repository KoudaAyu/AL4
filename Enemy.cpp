#include "Enemy.h"
#include "Wall.h"
#include "Random.h"
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
	if (!alive_) {
		// カウントダウンでリスポーン判定
		if (respawnCounter_ > 0) {
			--respawnCounter_;
		} else {
			// リスポーン
			alive_ = true;
			// ランダム位置に再配置（適宜範囲は調整）
			float x = Random::GeneratorFloat(-10.0f, 10.0f);
			float y = Random::GeneratorFloat(-10.0f, 10.0f);
			worldTransform_.translation_ = Vector3{x, y, 0.0f};
			// 初期速度をリセット
			speed = 0.0f;
		}
		// UpdateAABB と transform 更新はリスポーン後も行う
		UpdateAABB();
		worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
		worldTransform_.TransferMatrix();
		return;
	}

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

				const AABB& wallAABB = nearestWall->GetAABB();
				
				float wallHalfExtentX = (wallAABB.max.x - wallAABB.min.x) * 0.5f;
				float wallHalfExtentY = (wallAABB.max.y - wallAABB.min.y) * 0.5f;
				float wallHalfExtentZ = (wallAABB.max.z - wallAABB.min.z) * 0.5f;
				
				float wallExtent = (wallHalfExtentX > wallHalfExtentY) ? wallHalfExtentX : wallHalfExtentY;
				wallExtent = (wallExtent > wallHalfExtentZ) ? wallExtent : wallHalfExtentZ;

				
				float enemyExtent = (worldTransform_.scale_.x > worldTransform_.scale_.y) ? worldTransform_.scale_.x : worldTransform_.scale_.y;
				enemyExtent = (enemyExtent > worldTransform_.scale_.z) ? enemyExtent : worldTransform_.scale_.z;

				// 停止距離を少し小さめにして、AABBが確実に重なるようにする（衝突判定が確実に発生するように）
				float stopDistance = enemyExtent + wallExtent - 0.05f; // 少し重なる設定
				if (stopDistance < 0.01f) stopDistance = 0.01f; // 最小値を確保

				// 移動速度
				speed = 0.1f;                               // 移動速度（適宜調整）

				if (dirLen > stopDistance) {
					
					worldTransform_.translation_ += direction * speed; // Vector3の演算
				} else {
					// 停止位置は壁に少しめり込むように設定してAABB衝突が発生するようにする
					worldTransform_.translation_ = nearestWall->GetPosition() - direction * stopDistance;
					HandleCollision(); 
					speed = 0.0f;
				}
			}
		}
	}

	#pragma endregion 一番近くのWallに移動する処理

	UpdateAABB();

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();
}

void Enemy::Draw() { if (alive_ && model_) model_->Draw(worldTransform_, *camera_); }

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

void Enemy::HandleCollision() { speed = 0.0f; }

void Enemy::Kill() { alive_ = false; respawnCounter_ = kRespawnFrames; }
