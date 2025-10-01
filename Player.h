#pragma once
#include"KamataEngine.h"

enum class LRDirection
{ Left, Right };
;


class Player
{
public:
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3 position);
	void Update();
	void Draw();

	const KamataEngine::Vector3& GetPosition() const { return worldTransform_.translation_; }

private:

	KamataEngine::WorldTransform worldTransform_;

	KamataEngine::Model* model_ = nullptr;

	KamataEngine::Camera* camera_ = nullptr;

	uint32_t textureHandle_ = 0u;

	KamataEngine::Vector3 velocity_ = {};

	LRDirection lrDirection_ = LRDirection::Right;

	//慣性移動
	static inline const float kAcceleration = 0.05f;
};

