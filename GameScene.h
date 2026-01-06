#pragma once

#include "KamataEngine.h"

#include "CameraController.h"
#include "DeathParticle.h"
#include "Enemy.h"
#include "Player.h"
#include "Skydome.h"

#include <vector>

class MapChipField_;
class Spike;
class Goal;   // forward declaration
class Key;    // forward declaration for key objects
class Ladder; // forward declaration for ladder objects

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

	// Request flag accessor for returning to SelectScene from pause menu
	bool IsBackToSelectRequested() const { return backToSelectRequested_; }

public:
	int32_t GetWindowWidth() const { return kWindowWidth; }
	int32_t GetWindowHeight() const { return kWindowHeight; }

	bool IsFinished() { return finished_; }

	// Returns true when the player has entered the death sequence and the death animation finished
	bool IsPlayerDead() const { return readyForGameOver_; }

	// Request the player to ignore jump input on the next update (prevents carry-over jump after scene changes)
	void SuppressPlayerNextJump();

private:
	// Performs the actual immediate reset (called after fade-out completes)
	void PerformResetNow();

	bool finished_ = false;
	int startingStage_ = 0; // index of stage to load (set by SelectScene)

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

	// Flag indicating death animation finished and ready to go to GameOver
	bool readyForGameOver_ = false;

	std::vector<Enemy*> enemies_;

	std::vector<Spike*> spikes_;
	std::vector<Goal*> goals_;     // goal objects
	std::vector<Key*> keys_;       // key objects (multiple possible)
	std::vector<Ladder*> ladders_; // ladder objects
	Player* player_ = nullptr;

	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;

	Phase phase_ = Phase::kPlay;

	KamataEngine::Sprite* hudSprite_ = nullptr;
	uint32_t hudTextureHandle_ = 0u;

	KamataEngine::Sprite* uiLeftSprite_ = nullptr;
	uint32_t uiLeftTextureHandle_ = 0u;
	// Added: separate texture handles for left UI (gamepad RT and keyboard E)
	uint32_t uiLeftGamepadTextureHandle_ = 0u;
	uint32_t uiLeftKeyboardTextureHandle_ = 0u;
	uint32_t uiAttackTextureHandle_ = 0u; // for main left Attack UI
	KamataEngine::Sprite* uiMidSprite_ = nullptr;
	KamataEngine::Sprite* uiPhaseSprite_ = nullptr; // for victory/death mid UI
	KamataEngine::Sprite* uiAttackSprite_ = nullptr; // for main left Attack UI
	KamataEngine::Sprite* uiJumpSprite_ = nullptr;   // for jump icon on right UI
	uint32_t uiMidTextureHandle_ = 0u;
	KamataEngine::Sprite* uiRightSprite_ = nullptr;
	// Removed duplicate uiRightTextureHandle_. Use separate handles below.
	// Added: separate texture handles for gamepad and keyboard variants of right UI
	uint32_t uiRightGamepadTextureHandle_ = 0u;
	uint32_t uiRightKeyboardTextureHandle_ = 0u;
	uint32_t uiJumpTextureHandle_ = 0u; // for jump icon on right UI
	// Added: separate texture handles for gamepad/keyboard variants of mid UI
	uint32_t uiMidGamepadTextureHandle_ = 0u;
	uint32_t uiMidKeyboardTextureHandle_ = 0u;
	uint32_t uiPhaseTextureHandle_ = 0u; // for victory/death mid UI
    // Bottom-right UI sprite (uses left UI texture handles)
    KamataEngine::Sprite* uiBottomRightSprite_ = nullptr;

	// X position for middle UI (change this to shift mid UI left/right)
	float uiMidPosX_ = 90.0f; // default: slightly left of previous 120.0f

	// Pause UI
	KamataEngine::Sprite* pauseSprite_ = nullptr;
	uint32_t pauseTextureHandle_ = 0u;

	// New: pause menu sprites (three items shown top-down)
	KamataEngine::Sprite* pauseMenuSprites_[3] = {nullptr, nullptr, nullptr};
	uint32_t pauseMenuTextureHandles_[3] = {0u, 0u, 0u};
	int pauseMenuSelectedIndex_ = 0;

	// Request to go back to SelectScene
	bool backToSelectRequested_ = false;

	// Request to resume next frame (prevents immediate input passing to player)
	bool resumeRequested_ = false;

	// Remember last input device: true if last input was gamepad, false for keyboard
	bool lastInputWasPad_ = false;

	// HP hearts UI
	struct HeartUI {
		KamataEngine::Sprite* sprite = nullptr;
		float baseSize = 32.0f; // starting size when remove animation begins
		float currentSize = 32.0f;
		float animTimer = 0.0f;
		bool removing = false;
		KamataEngine::Vector2 startPos = {0.0f, 0.0f}; // initial on-screen position
	};
	std::vector<HeartUI> hearts_;
	uint32_t heartTextureHandle_ = 0u;
	// duration for heart removal animation in seconds
	float heartRemoveDuration_ = 0.25f;
	// keep last known player HP to detect changes
	int lastPlayerHP_ = 0;

	float victoryTimer_ = 0.0f;
	// duration for the victory sequence (seconds)
	float victoryDuration_ = 2.0f;

	KamataEngine::Sprite* countdownSprite_ = nullptr;
	uint32_t countdownTextureHandles_[10] = {0};
	float countdownTime_ = 0.0f;
	float countdownStart_ = 3.0f;

	class Fade* fade_ = nullptr;
	float introFadeDuration_ = 1.0f;

	// Deferred reset state: when true a fade-out is in progress and the actual reset will be
	// performed once the fade finishes.
	bool resetPending_ = false;
	float resetFadeDuration_ = 0.6f; // seconds

	uint32_t seDecisionDataHandle_ = 0u;

	uint32_t bgmDataHandle_ = 0u;
	uint32_t bgmVoiceHandle_ = 0u;
	bool bgmStarted_ = false;
};