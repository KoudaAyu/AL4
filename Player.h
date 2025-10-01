#pragma once
#include"KamataEngine.h"

class Player
{
public:
	void Initialize(KamataEngine::Model* model, uint32_t texture,KamataEngine::Camera* camera);
	void Uppdate();
	void Draw();

private:

	KamataEngine::WorldTransform worldTransform_;

	KamataEngine::Model* model_ = nullptr;

	KamataEngine::Camera* camera_ = nullptr;

	uint32_t textureHandle_ = 0u;
};

