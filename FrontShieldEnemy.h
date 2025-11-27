#pragma once

#include "Enemy.h"
#include "KamataEngine.h"
#include <string>

class FrontShieldEnemy : public Enemy {

public:
	enum class LRDirection {
		kRight,
		kLeft,
	};

public:
	FrontShieldEnemy() = default;
	~FrontShieldEnemy() override = default;

	using Enemy::Initialize;


	void OnCollision(Player* player) override;

	
	void SetFrontDotThreshold(float t) { frontDotThreshold_ = t; }

	void Initialize(KamataEngine::Camera* camera, const KamataEngine::Vector3& pos);

private:

	LRDirection lrDirection_ = LRDirection::kLeft;

	float frontDotThreshold_ = 0.5f;
};