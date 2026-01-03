#pragma once

#include "KamataEngine.h"
#include "../AABB.h"

class Player;

class Enemy {
public:
	Enemy() = default;
	virtual ~Enemy();
	void Initialize(KamataEngine::Camera* camera, const KamataEngine::Vector3& pos);
	virtual void Update();
	virtual void Draw();

	void UpdateAABB();

	virtual void OnCollision(Player* player);

public:
	AABB& GetAABB() { return aabb_; }

	bool isAlive() const { return isAlive_; }

protected:

	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;

	AABB aabb_;

	bool isAlive_ = true;

	bool ownsModel_ = false;
};