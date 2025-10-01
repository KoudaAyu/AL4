#pragma once

#include "KamataEngine.h"

#include"Player.h"

class GameScene {
public:

	GameScene();
	~GameScene();

	void Initialize();
	void Update();
	void Draw();

	int32_t GetWindowWidth() const { return kWindowWidth; }
	int32_t GetWindowHeight() const { return kWindowHeight; }

private:
	const int32_t kWindowWidth = 1280;
	const int32_t kWindowHeight = 720;

	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera camera_;

	uint32_t textureHandle_ = 0u;

	//デバックカメラ
	KamataEngine::DebugCamera* debugCamera_ = nullptr;

	Player* player_ = nullptr;
};