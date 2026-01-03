#include "DeathParticle.h"


#include<algorithm>
#include"MathUtl.h"

using namespace KamataEngine;

DeathParticle::DeathParticle() {
	std::random_device rd;
	rnd_.seed(rd());
}

DeathParticle::~DeathParticle() {}

void DeathParticle::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& pos) {
#ifdef _DEBUG
	assert(model);
#endif
	// 引数として受け取ったデータをメンバ関数に記録
	model_ = model;
	/*textureHandle_ = textureHandle;*/
	camera_ = camera;

	// initialize each particle
	std::uniform_real_distribution<float> angDist(0.0f, 2.0f * std::numbers::pi_v<float>);
	std::uniform_real_distribution<float> speedDist(0.5f * kBaseSpeed, 1.6f * kBaseSpeed);
	std::uniform_real_distribution<float> scaleDist(0.6f, 1.6f);
	std::uniform_real_distribution<float> rotDist(-3.0f, 3.0f);
	std::uniform_real_distribution<float> colorJitter(-0.15f, 0.15f);

	for (uint32_t i = 0; i < kNumParticles; ++i) {
		worldTransforms_[i].Initialize();
		worldTransforms_[i].translation_ = pos;

		float angle = angDist(rnd_);
		float speed = speedDist(rnd_);
		Vector3 dir = {std::cos(angle), std::sin(angle), (std::uniform_real_distribution<float>(-0.2f,0.2f))(rnd_)};
		velocities_[i] = Normalize(dir) * speed;

		angularVel_[i] = rotDist(rnd_);
		initScale_[i] = scaleDist(rnd_);
		worldTransforms_[i].scale_ = {initScale_[i], initScale_[i], initScale_[i]};

		// per-particle color variation
		float r = std::clamp(baseColor_.x + colorJitter(rnd_), 0.0f, 1.0f);
		float g = std::clamp(baseColor_.y + colorJitter(rnd_), 0.0f, 1.0f);
		float b = std::clamp(baseColor_.z + colorJitter(rnd_), 0.0f, 1.0f);
		colorVals_[i] = {r, g, b, 1.0f};
		objectColors_[i].Initialize();
		objectColors_[i].SetColor(colorVals_[i]);
	}

	isFinish_ = false;
	counter_ = 0.0f;
}


void DeathParticle::Update() {

	if (isFinish_)
	{
		return;
	}

	counter_ += 1.0f / 60.0f;
	if (counter_ >= kDuration) {
		counter_ = kDuration;
		isFinish_ = true;
	}

	float t = counter_ / kDuration; 

	for (uint32_t i = 0; i < kNumParticles; ++i) {
		// gravity-like effect
		velocities_[i].y -= 0.004f; // small downward pull
		// slow down slightly over time
		velocities_[i] *= 0.995f;

		worldTransforms_[i].translation_ += velocities_[i];

		// rotate particle
		worldTransforms_[i].rotation_.z += angularVel_[i] * (1.0f / 60.0f);

		// scale easing out
		float scale = initScale_[i] * (1.0f + 0.6f * (1.0f - t));
		worldTransforms_[i].scale_ = {scale, scale, scale};

		// fade out
		colorVals_[i].w = std::clamp(1.0f - t, 0.0f, 1.0f);
		objectColors_[i].SetColor(colorVals_[i]);

		worldTransforms_[i].matWorld_ = MakeAffineMatrix(worldTransforms_[i].scale_, worldTransforms_[i].rotation_, worldTransforms_[i].translation_);
		worldTransforms_[i].TransferMatrix();
	}

}


void DeathParticle::Draw() { 

	if (isFinish_)
	{
		return;
	}
	for (uint32_t i = 0; i < kNumParticles; ++i) {
		model_->Draw(worldTransforms_[i], *camera_, &objectColors_[i]);
	}
}

void DeathParticle::emitOctagonalParticles() {
	// legacy: not used now
}
