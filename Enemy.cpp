#include "Enemy.h"
#include "Wall.h"
#include "Random.h"
#include "HealerActor.h"
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

void Enemy::Update(const std::list<Wall*>& walls, const std::list<HealerActor*>& healers) {
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

	// まずは修復中のHealerActorの位置を優先して狙う
	HealerActor const* targetHealer = nullptr;
	float bestHealerDist = FLT_MAX;
	for (HealerActor const* ha : healers) {
		if (!ha) continue;
		if (!ha->IsAssigned()) continue; // 修復中のもののみ
		float d = (ha->GetPosition() - worldTransform_.translation_).Length();
		if (d < bestHealerDist) { bestHealerDist = d; targetHealer = ha; }
	}

	KamataEngine::Vector3 targetPos{0,0,0};
	bool hasTarget = false;
	if (targetHealer) {
		hasTarget = true;
		targetPos = targetHealer->GetPosition();
	} else {
		// 修復中のHealerがいなければ従来通り最寄りのWallを狙う
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

		if (nearestWall) {
			hasTarget = true;
			targetPos = nearestWall->GetPosition();
		}
	}

	if (hasTarget) {
		Vector3 direction = targetPos - worldTransform_.translation_;
		float dirLen = Length(direction);
		
		if (dirLen > 0.0f) {
			direction = Normalize(direction);

			// 移動速度
			speed = 0.1f;                               // 移動速度（適宜調整）

			worldTransform_.translation_ += direction * speed; // Vector3の演算
		}
	}

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
