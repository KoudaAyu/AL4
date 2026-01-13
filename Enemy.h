#pragma once
#include "KamataEngine.h"
#include "MathUtl.h"

class Enemy {
public:
	Enemy();
	~Enemy();
	void Initialize(KamataEngine::Camera* camera, KamataEngine::Vector3 pos);
	void Update();
	void Draw();

	void UpdateAABB();

	const AABB& GetAABB() const;

	void HandleCollision();

private:
	AABB aabb_{};

private:
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Camera* camera_ = nullptr;
};