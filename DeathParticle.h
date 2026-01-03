#pragma once
#include "KamataEngine.h"

#include<array>
#include<numbers>
#include <random>

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
	static inline const uint32_t kNumParticles = 32; // more particles for flashier effect

	// 存続時間(消滅までの時間)<秒>
	static inline const float kDuration = 2.0f;

	// 基本の移動の速さ
	static inline const float kBaseSpeed = 0.25f;

	// 終了フラグ
	bool isFinish_ = false;

	// 経過時間
	float counter_ = 0.0f;

private:

	std::array<KamataEngine::WorldTransform, kNumParticles> worldTransforms_{};
	std::array<KamataEngine::Vector3, kNumParticles> velocities_{};
	std::array<float, kNumParticles> angularVel_{};
	std::array<float, kNumParticles> initScale_{};
	std::array<KamataEngine::ObjectColor, kNumParticles> objectColors_{};
	std::array<KamataEngine::Vector4, kNumParticles> colorVals_{};
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;

	// common color (base)
	KamataEngine::Vector4 baseColor_ = {1.0f, 0.9f, 0.4f, 1.0f}; // warm burst by default

	std::mt19937 rnd_;
};
