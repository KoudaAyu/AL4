#pragma once
#include "KamataEngine.h"

#include <list>

#include "Wall.h"
#include "Enemy.h"
#include "Player.h"
#include "Healer.h"

class GameScene {
public:
	GameScene();
	~GameScene();

	void Initialize();

	void Update();

	void Draw();

	void CollisionCheck();

private:
	std::list<Wall*> walls_;
	static inline const int32_t kMaxWall_ = 16;

	std::list<Enemy*> enemies_;
	static inline const int32_t kMaxEnemy_ = 5;

	Player* player_ = nullptr;

	Healer* healer_ = nullptr;

private:
	KamataEngine::Model* model_ = nullptr;

	KamataEngine::WorldTransform worldTransform_;

	KamataEngine::Camera camera_;
};