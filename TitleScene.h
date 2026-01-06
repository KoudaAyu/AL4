#pragma once
#include "KamataEngine.h"

#include"Fade.h"
#include "DeathParticle.h"

class TitleScene {

	public:
		enum class Phase
		{
			kFadeIn,
			kMain,
			kEffect,
			kFadeOut,
	    };

public:

	TitleScene() = default;
	~TitleScene();

	void Initialize();
	void Update();
	void Draw();

public:
	bool IsFinished() { return finished_; }

private:
	bool finished_ = false;

	private:
	Fade* fade_ = nullptr;

	Phase phase_ = Phase::kFadeIn;

	// 3D title model
	KamataEngine::Model* model_ = nullptr;
	// model used for particles
	KamataEngine::Model* particleModel_ = nullptr;
	KamataEngine::Camera camera_;
	KamataEngine::WorldTransform worldTransform_;

	// effect (演出) state - particle & rotation based
	float effectTimer_ = 0.0f;
	float effectDuration_ = 1.0f; // seconds for the effect animation
	DeathParticle* particle_ = nullptr;
	float rotationBoost_ = 6.0f; // additional rotation speed multiplier when boosted
	float cameraStartZ_ = -50.0f;

	// rotation speed control (radians per second)
	float rotationSpeed_ = 0.5f; // current rotation speed (rad/s)
	float targetRotationSpeed_ = 0.5f; // desired rotation speed
	float rotationLerpSpeed_ = 8.0f; // how fast rotationSpeed_ approaches target (per second)

	// Skydome for background
	class Skydome* skydome_ = nullptr;

	// Audio handles
	uint32_t bgmDataHandle_ = 0u;
	uint32_t bgmVoiceHandle_ = 0u;
	uint32_t seDecisionDataHandle_ = 0u;
	bool bgmStarted_ = false;

};