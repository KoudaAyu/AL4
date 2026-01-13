#pragma once
#include "KamataEngine.h"

#include<list>

#include "BloodVessels.h"
#include"Enemy.h"
#include "Player.h"

class GameScene {
public:
	GameScene();
	~GameScene();

	void Initialize();

	void Update();

	void Draw();

	void CollisionCheck();

private:

	BloodVessels* bloodVessels_ = nullptr;


	std::list<Enemy*> enemies_;
	static inline const int32_t kMaxEnemy_ = 5;

	Player* player_ = nullptr;

private:
	KamataEngine::Model* model_ = nullptr;

	KamataEngine::WorldTransform worldTransform_;

	KamataEngine::Camera camera_;
};