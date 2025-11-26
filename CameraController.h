#pragma once
#include "KamataEngine.h"

class Player;

/// <summary>
/// 左、右、上、下の順に値をいれる
/// </summary>
struct Rect {
	float left = 0.0f;
	float right = 1.0f;
	float top = 1.0f;
	float bottom = 0.0f;
};

class CameraController {
public:

	CameraController();
	~CameraController();

	// シーンのカメラを受け取る
	void Initialize(KamataEngine::Camera* camera);
	void Update();
	void Draw();

	void Reset();

	/// <summary>
	/// 
	/// </summary>
	/// <param name="area">左、右、上、下の順に値をいれる(マップ全体サイズ)</param>
	void SetMovableArea(const Rect& area) { movableArea = area; }

public:
	void SetTarget(Player* target) { target_ = target; }

	// カメラシェイクを開始する
	// amplitude: 揺れ幅（ワールド座標単位）
	// duration: 継続時間（秒）
	void StartShake(float amplitude, float duration);

private:
	KamataEngine::Vector3 targetOffset_ = {0.0f, 0.0f, -15.0f};
	KamataEngine::Vector3 targetVelocity_ = {};
	Rect movableArea = {0, 100, 0, 100};
	static inline const Rect Margin = {-10.0f, 10.0f, 10.0f, -10.0f};

private:
	KamataEngine::Camera* camera_ = nullptr; // シーンのカメラを参照
	Player* target_ = nullptr;

	KamataEngine::Vector3 targetPosition_ = {};

	//座標補間割合
	static inline const float kInterpolationRate = 1.0f;

	//速度掛け率
	static inline const float kVelocityBias = 0.1f;

	// --- カメラシェイク用 ---
	float shakeAmplitude_ = 0.0f;    // 初期振幅
	float shakeDuration_ = 0.0f;     // 総継続時間
	float shakeRemaining_ = 0.0f;    // 残り時間
	bool isShaking_ = false;
};