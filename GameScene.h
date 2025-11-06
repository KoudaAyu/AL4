#pragma once

#include "KamataEngine.h"

#include "CameraController.h"
#include"Player.h"

#include<vector>

class GameScene {
public:

	GameScene();
	~GameScene();

	void Initialize();
	void Update();
	void Draw();

public:

	int32_t GetWindowWidth() const { return kWindowWidth; }
	int32_t GetWindowHeight() const { return kWindowHeight; }

	bool IsFinished() { return finished_; }

private:
	bool finished_ = false;

	//ブロックの要素数
	const uint32_t kNumBlockHorizontal = 20;
	const uint32_t kNumBlockVertical = 10;
	//ブロック一つのサイズ
	const float kBlockWidth = 2.0f;
	const float kBlockHeight = 2.0f;

private:
	const int32_t kWindowWidth = 1280;
	const int32_t kWindowHeight = 720;

	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Model* blockModel_ = nullptr;
	KamataEngine::Camera camera_;

	uint32_t textureHandle_ = 0u;

	//デバックカメラ
	KamataEngine::DebugCamera* debugCamera_ = nullptr;
	bool isDebugCameraActive_ = false;

	CameraController* cameraController_ = nullptr;
	Player* player_ = nullptr;

	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;
};