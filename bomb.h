#pragma once

#include "KamataEngine.h"

class Bomb {

public:
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);
	void Update();
	void Draw();

	const KamataEngine::Vector3& GetPosition() const { return worldTransform_.translation_; }

private:
	bool isAlive_ = true;

	uint32_t textureHandle_ = 0u;


	KamataEngine::WorldTransform worldTransform_;

	KamataEngine::Model* model_ = nullptr;

	KamataEngine::Camera* camera_ = nullptr;

};