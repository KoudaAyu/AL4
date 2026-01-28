#pragma once

#include "KamataEngine.h"
#include "../AABB.h"

class Player;
class MapChipField;

class Enemy {
public:
	Enemy() = default;
	virtual ~Enemy();
	// Add facing parameter: faceLeft = true makes enemy face left on spawn
	void Initialize(KamataEngine::Camera* camera, const KamataEngine::Vector3& pos);
	void Initialize(KamataEngine::Camera* camera, const KamataEngine::Vector3& pos, bool faceLeft);
	virtual void Update();
	virtual void Draw();

	void UpdateAABB();

	virtual void OnCollision(Player* player);

	// Movement helpers
	void SetMapChipField(MapChipField* map);
	void SetFacingRight(bool facing);
	void SetSpeed(float s) { speed_ = s; velocityX_ = (facingRight_ ? speed_ : -speed_); }

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

	// Movement state
	MapChipField* mapChipField_ = nullptr;
	float speed_ = 0.12f; // units per frame (simple constant since Update called without dt)
	float velocityX_ = 0.0f;
	bool facingRight_ = false;
};