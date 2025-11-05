#pragma once
#include "KamataEngine.h"

class Player;

class CameraController {

public:
	CameraController();
	~CameraController();

	// シーンのカメラを受け取る
	void Initialize(KamataEngine::Camera* camera);
	void Update();
	void Draw();

	void Reset();

public:
	void SetTarget(Player* target) { target_ = target; }

private:
	KamataEngine::Vector3 targetOffset_ = {0.0f, 0.0f, -15.0f};

private:
	KamataEngine::Camera* camera_ = nullptr; // シーンのカメラを参照
	Player* target_ = nullptr;
};