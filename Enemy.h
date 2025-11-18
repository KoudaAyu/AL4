#pragma once

#include "KamataEngine.h"

class Enemy {
public:
	Enemy() = default;
	~Enemy();
	void Initialize();
	void Update();
	void Draw();

private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;
};