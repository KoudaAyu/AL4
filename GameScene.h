#pragma once
#include "KamataEngine.h"
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
	Enemy* enemy_ = nullptr;
	Player* player_ = nullptr;

private:
	KamataEngine::Model* model_ = nullptr;

	KamataEngine::WorldTransform worldTransform_;

	KamataEngine::Camera camera_;
};