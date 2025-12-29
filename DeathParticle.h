#pragma once
#include "KamataEngine.h"

#include<array>
#include<numbers>

class DeathParticle {

public:
	DeathParticle();
	~DeathParticle();
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& pos);
	void Update();
	void Draw();

	void emitOctagonalParticles();

	// Return true when particle has finished its life
	bool IsFinished() const { return isFinish_; }

private:
	// Particleの数
	static inline const uint32_t kNumParticles = 8;

	// 存続時間(消滅までの時間)<秒>
	static inline const float kDuration = 2.0f;

	// 移動の速さ
	static inline const float kSpeed = 0.1f;

	// 分割した1個分の角度
	static inline const float kAngleUnit = std::numbers::pi_v<float> * 2.0f / 8.0f;

	// 終了フラグ
	bool isFinish_ = false;

	// 経過時間
	float counter_ = 0.0f;

private:

	std::array<KamataEngine::WorldTransform, kNumParticles> worldTransforms_{};
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;

	KamataEngine::ObjectColor objectColor_;

	KamataEngine::Vector4 color_;
};
