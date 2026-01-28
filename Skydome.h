#pragma once

#include "KamataEngine.h"

class Skydome {
public:
	void Initialize();
	void Update();
	void Draw();

	// カメラの設定
	void SetCamera(KamataEngine::Camera* camera) { camera_ = camera; }

	// スカイドーム回転速度の設定（ラジアン/秒）
	void SetRotationSpeed(float s) { rotationSpeed_ = s; }

private:
	KamataEngine::Camera* camera_ = nullptr;
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;

	float rotationSpeed_ = 0.0f; // radians per second
};
