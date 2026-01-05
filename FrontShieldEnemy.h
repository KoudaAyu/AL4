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
	~FrontShieldEnemy() override;

	using Enemy::Initialize;


	void OnCollision(Player* player) override;
	void Update() override;
	void Draw() override;

	
	void SetFrontDotThreshold(float t) { frontDotThreshold_ = t; }

	void Initialize(KamataEngine::Camera* camera, const KamataEngine::Vector3& pos);

private:

	LRDirection lrDirection_ = LRDirection::kLeft;

	float frontDotThreshold_ = 0.5f;

	uint32_t soundDataHandle = KamataEngine::Audio::GetInstance()->LoadWave("Audio/SE/FrontShieldEnemy_Alive.wav");

	// Separate shield model so shield and body can be transformed independently
	KamataEngine::Model* shieldModel_ = nullptr;
	bool ownsShieldModel_ = false;
	KamataEngine::WorldTransform shieldWorldTransform_;
};