#pragma once

#include "KamataEngine.h"

#include "CameraController.h"
#include "DeathParticle.h"
#include "Enemy.h"
#include "Player.h"
#include "Skydome.h"

#include <vector>

class MapChipField_;

class GameScene {
public:
	enum class Phase {
		kPlay,
		kDeath,
	};

public:
	GameScene();
	~GameScene();

	void Initialize();
	void Update();
	void Draw();

	void GenerateBlocks();

	/// <summary>
	/// マップチップ以外の当たり判定をすべてチェックする
	/// </summary>
	void CheckAllCollisions();

	/// <summary>
	/// フェーズの切り替え
	/// </summary>
	void ChangePhase();

	// リセット
	void Reset();

public:
	int32_t GetWindowWidth() const { return kWindowWidth; }
	int32_t GetWindowHeight() const { return kWindowHeight; }

	bool IsFinished() { return finished_; }

private:
	bool finished_ = false;

	// ブロックの要素数
	const uint32_t kNumBlockHorizontal = 20;
	const uint32_t kNumBlockVertical = 10;
	// ブロック一つのサイズ
	const float kBlockWidth = 2.0f;
	const float kBlockHeight = 2.0f;

private:
	const int32_t kWindowWidth = 1280;
	const int32_t kWindowHeight = 720;

	KamataEngine::Model* model_ = nullptr;      // デバック用
	KamataEngine::Model* blockModel_ = nullptr; // ブロック用
	Skydome* skydome_ = nullptr;                // スカイドームクラス
	KamataEngine::Camera camera_;

	uint32_t textureHandle_ = 0u;

	// デバックカメラ
	KamataEngine::DebugCamera* debugCamera_ = nullptr;
	bool isDebugCameraActive_ = false;

	CameraController* cameraController_ = nullptr;
	MapChipField* mapChipField_ = nullptr;

	// Particle関係
	DeathParticle* deathParticle_ = nullptr;

	// Support multiple enemies
	std::vector<Enemy*> enemies_;
	Player* player_ = nullptr;

	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;

	Phase phase_ = Phase::kPlay;
};