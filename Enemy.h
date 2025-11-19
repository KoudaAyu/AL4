#pragma once

#include"AABB.h"
#include "KamataEngine.h"

class Player;

class Enemy {
public:
	Enemy() = default;
	~Enemy();
	void Initialize(KamataEngine::Model* model,KamataEngine::Camera* camera, const KamataEngine::Vector3& pos);
	void Update();
	void Draw();

	void UpdateAABB();

	void OnCollision(Player* player);

public:
	AABB& GetAABB() { return aabb_; }

private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;

	AABB aabb_;
};