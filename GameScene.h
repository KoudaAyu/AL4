#pragma once

#include "KamataEngine.h"

#include "CameraController.h"
#include "DeathParticle.h"
#include "Enemy.h"
#include"EnemyDeathParticle.h"
#include "Player.h"
#include "Skydome.h"

#include <vector>

class MapChipField_;
class Spike;
class Goal;  
class Key;   
class Ladder; 

class GameScene {
public:
	enum class Phase {
		kCountdown,
		kPlay,
		kDeath,
		kPause,   // 追加: 一時停止
		kVictory, // 新規: クリア演出フェーズ（すぐにシーン遷移しない）
	};

public:
	GameScene();
	explicit GameScene(int startingStage);
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

	
	bool IsBackToSelectRequested() const { return backToSelectRequested_; }

public:
	int32_t GetWindowWidth() const { return kWindowWidth; }
	int32_t GetWindowHeight() const { return kWindowHeight; }

	bool IsFinished() { return finished_; }

	
	bool IsPlayerDead() const { return readyForGameOver_; }

	
	void SuppressPlayerNextJump();

private:
	
	void PerformResetNow();

	bool finished_ = false;
	int startingStage_ = 0; 

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
	KamataEngine::Model* iceModel_ = nullptr;   // 氷ブロック用
	KamataEngine::Model* nikukyuModel_ = nullptr; // 肉球モデル（デスパーティクル用）
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
	std::vector<EnemyDeathParticle*> enemyDeathParticles_;
	
	bool readyForGameOver_ = false;

	std::vector<Enemy*> enemies_;

	std::vector<Spike*> spikes_;
	std::vector<Goal*> goals_;     
	std::vector<Key*> keys_;       
	std::vector<Ladder*> ladders_; 
	Player* player_ = nullptr;

	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;

	Phase phase_ = Phase::kPlay;

	KamataEngine::Sprite* hudSprite_ = nullptr;
	uint32_t hudTextureHandle_ = 0u;

	KamataEngine::Sprite* uiLeftSprite_ = nullptr;
	uint32_t uiLeftTextureHandle_ = 0u;
	
	uint32_t uiLeftGamepadTextureHandle_ = 0u;
	uint32_t uiLeftKeyboardTextureHandle_ = 0u;
	uint32_t uiAttackTextureHandle_ = 0u;
	KamataEngine::Sprite* uiMidSprite_ = nullptr;
	KamataEngine::Sprite* uiPhaseSprite_ = nullptr;
	KamataEngine::Sprite* uiAttackSprite_ = nullptr;
	KamataEngine::Sprite* uiJumpSprite_ = nullptr;
	uint32_t uiMidTextureHandle_ = 0u;
	KamataEngine::Sprite* uiRightSprite_ = nullptr;
	
	uint32_t uiRightGamepadTextureHandle_ = 0u;
	uint32_t uiRightKeyboardTextureHandle_ = 0u;
	uint32_t uiJumpTextureHandle_ = 0u;
	
	uint32_t uiMidGamepadTextureHandle_ = 0u;
	uint32_t uiMidKeyboardTextureHandle_ = 0u;
	uint32_t uiPhaseTextureHandle_ = 0u; 
    
    KamataEngine::Sprite* uiBottomRightSprite_ = nullptr;

	
	float uiMidPosX_ = 90.0f; 

	
	KamataEngine::Sprite* pauseSprite_ = nullptr;
	uint32_t pauseTextureHandle_ = 0u;

	
	KamataEngine::Sprite* pauseMenuSprites_[3] = {nullptr, nullptr, nullptr};
	uint32_t pauseMenuTextureHandles_[3] = {0u, 0u, 0u};
	int pauseMenuSelectedIndex_ = 0;

	
	bool backToSelectRequested_ = false;

	
	bool resumeRequested_ = false;

	
	bool lastInputWasPad_ = false;

	
	struct HeartUI {
		KamataEngine::Sprite* sprite = nullptr;
		float baseSize = 32.0f; 
		float currentSize = 32.0f;
		float animTimer = 0.0f;
		bool removing = false;
		KamataEngine::Vector2 startPos = {0.0f, 0.0f};
	};
	std::vector<HeartUI> hearts_;
	uint32_t heartTextureHandle_ = 0u;
	
	float heartRemoveDuration_ = 0.25f;
	
	int lastPlayerHP_ = 0;

	float victoryTimer_ = 0.0f;
	
	float victoryDuration_ = 2.0f;

	KamataEngine::Sprite* countdownSprite_ = nullptr;
	uint32_t countdownTextureHandles_[10] = {0};
	float countdownTime_ = 0.0f;
	float countdownStart_ = 3.0f;

	class Fade* fade_ = nullptr;
	float introFadeDuration_ = 1.0f;


	bool resetPending_ = false;
	float resetFadeDuration_ = 0.6f; 

	uint32_t seDecisionDataHandle_ = 0u;

	uint32_t bgmDataHandle_ = 0u;
	uint32_t bgmVoiceHandle_ = 0u;
	bool bgmStarted_ = false;

	uint32_t seClearDataHandle_ = 0u;

	 // 左下UI用スプライト（Resources/Sprite/SelectScene/LT.png を想定）
	uint32_t ltTexHandle_ = 0u;
	KamataEngine::Sprite* ltSprite_ = nullptr;

	// キーボード用Qアイコン
	uint32_t qTexHandle_ = 0u;
	KamataEngine::Sprite* qSprite_ = nullptr;

	 enum class InputMode { kUnknown = 0, kGamepad, kKeyboard };
	InputMode lastInputMode_ = InputMode::kUnknown;
};