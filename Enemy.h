#pragma once
#include "KamataEngine.h"
#include "MathUtl.h"
#include <list>

class Wall;

class Enemy {
public:
	Enemy();
	~Enemy();
	void Initialize(KamataEngine::Camera* camera, KamataEngine::Vector3 pos);

	void Update(const std::list<Wall*>& walls);
	void Draw();

	void UpdateAABB();

	const AABB& GetAABB() const;

	void HandleCollision();

	bool IsAlive() const { return alive_; }
	void Kill() { alive_ = false; }

private:
	AABB aabb_{};

	float speed;

	bool alive_ = true;

private:
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Camera* camera_ = nullptr;
};