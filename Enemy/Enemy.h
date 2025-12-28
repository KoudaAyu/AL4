#pragma once


#include "KamataEngine.h"
#include "../AABB.h"
class Player;

class Enemy {
public:
	Enemy() = default;
	virtual ~Enemy();
	void Initialize(KamataEngine::Camera* camera, const KamataEngine::Vector3& pos);
	void Update();
	void Draw();

	void UpdateAABB();

	virtual void OnCollision(Player* player);

public:
	AABB& GetAABB() { return aabb_; }

	bool isAlive() const { return isAlive_; }

protected:
	// moved to protected so subclasses can access transform/rotation
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;

	AABB aabb_;

	bool isAlive_ = true;

	// If true, this Enemy instance owns the Model pointer and will delete it in the destructor.
	// Subclasses that create their own Model should set this to true (similar to Player::ownsModel_).
	bool ownsModel_ = false;
};