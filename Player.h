#pragma once
#define NOMINMAX
#include <Windows.h>

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

	// 向き
	LRDirection lrDirection_ = LRDirection::Right;

	//慣性移動
	static inline const float kAcceleration = 0.05f;

	//速度減少率
	static inline const float kAttenuation = 0.1f;

	//速度制限
	static inline const float kLimitSpeed = 2.0f;

	//旋回開始時の角度
	float turnFirstRotationY_ = 0.0f;

	//旋回タイマー
	float turnTimer_ = 0.0f;

	//旋回にかかる時間<秒>
	static inline const float kTimeTurn = 0.3f;

	// ジャンプ
	bool isJump_ = false;
	float jumpVelocity_ = 0.0f;
	static inline float kGravity = 0.2f;
	static inline float kJumpVelocity = 2.0f;
	// ジャンプ回数
	int jumpCount_ = 0;
	// 最大ジャンプ回数（二段ジャンプなら2）
	static inline const int kMaxJumpCount = 2;
};

