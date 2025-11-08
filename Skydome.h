#pragma once

#include "KamataEngine.h"

class Skydome {
public:
	void Initialize();
	void Update();
	void Draw();

	// カメラの設定
	void SetCamera(KamataEngine::Camera* camera) { camera_ = camera; }

private:
	KamataEngine::Camera* camera_ = nullptr;
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
};
