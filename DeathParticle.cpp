#include "DeathParticle.h"


#include<algorithm>
#include"MathUtl.h"

using namespace KamataEngine;

DeathParticle::DeathParticle() {
	std::random_device rd;
	rnd_.seed(rd());
}

DeathParticle::~DeathParticle() {}

void DeathParticle::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& pos, bool isVictory) {
#ifdef _DEBUG
	assert(model);
#endif
	// 引数として受け取ったデータをメンバ関数に記録
	model_ = model;
	/*textureHandle_ = textureHandle;*/
	camera_ = camera;
	isVictory_ = isVictory;

	// Adjust base appearance depending on mode
	if (isVictory_) {
		// vibrant celebratory tone
		baseColor_ = {0.85f, 0.95f, 1.0f, 1.0f};
		// make victory effect last longer
		duration_ = kDuration * 1.5f;
	} else {
		// warm burst
		baseColor_ = {1.0f, 0.9f, 0.4f, 1.0f};
		duration_ = kDuration;
	}

	// initialize each particle
	std::uniform_real_distribution<float> angDist(0.0f, 2.0f * std::numbers::pi_v<float>);
	std::uniform_real_distribution<float> speedDist(
		isVictory_ ? (0.5f * kBaseSpeed) : (0.6f * kBaseSpeed),
		isVictory_ ? (2.2f * kBaseSpeed) : (1.6f * kBaseSpeed)
	);
	std::uniform_real_distribution<float> scaleDist(
		isVictory_ ? 0.4f : 0.6f,
		isVictory_ ? 1.8f : 1.4f
	);
	std::uniform_real_distribution<float> rotDist(
		isVictory_ ? -2.0f : -3.0f,
		isVictory_ ? 2.0f : 3.0f
	);
	std::uniform_real_distribution<float> colorJitter(
		isVictory_ ? -0.25f : -0.15f,
		isVictory_ ? 0.25f : 0.15f
	);

	for (uint32_t i = 0; i < kNumParticles; ++i) {
		worldTransforms_[i].Initialize();
		worldTransforms_[i].translation_ = pos;

		float angle = angDist(rnd_);
		float speed = speedDist(rnd_);
		Vector3 dir = { std::cos(angle), std::sin(angle), (std::uniform_real_distribution<float>(-0.10f,0.40f))(rnd_) };
		// In victory mode, bias upward and add star-burst shells
		if (isVictory_) {
			// create two distinct behaviors: inner floaters and outer rays
			bool outerRay = (i % 4 == 0);
			if (outerRay) {
				// outer rays: stronger upward and outward
				dir.y = std::abs(dir.y) + 0.6f;
				speed *= 1.6f;
				initScale_[i] = (std::uniform_real_distribution<float>(0.6f, 1.2f))(rnd_);
			} else {
				// inner floaters: gentle upward drift
				dir.y = std::abs(dir.y) + 0.2f;
			}
		}
		velocities_[i] = Normalize(dir) * speed;

		angularVel_[i] = rotDist(rnd_);
		// if not set by outerRay branch above, randomize scale here
		if (!isVictory_ || (isVictory_ && (i % 4 != 0))) {
			initScale_[i] = scaleDist(rnd_);
		}
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

	const float dt = 1.0f / 60.0f;
	counter_ += dt;
	if (counter_ >= duration_) {
		counter_ = duration_;
		isFinish_ = true;
	}

	float t = counter_ / duration_;

	for (uint32_t i = 0; i < kNumParticles; ++i) {
		if (isVictory_) {
			// two behaviors: outer rays keep speed, inner floaters drift with lateral swirl
			bool outerRay = (i % 4 == 0);
			if (!outerRay) {
				float swirl = std::sin(counter_ * (2.5f + 0.3f * (i % 7))) * 0.03f;
				velocities_[i].x += swirl;
				// gentle upward bias
				velocities_[i].y += 0.002f;
				// very light gravity
				velocities_[i].y -= 0.0006f;
				// minimal damping
				velocities_[i] *= 0.9995f;
			} else {
				// outer rays: keep momentum, slight spread over time
				velocities_[i] *= 0.999f;
			}
		} else {
			// explosion fall
			velocities_[i].y -= 0.004f; // small downward pull
			velocities_[i] *= 0.995f;    // slight damping
		}

		worldTransforms_[i].translation_ += velocities_[i];

		// rotate particle
		worldTransforms_[i].rotation_.z += angularVel_[i] * dt;

		// scale evolution
		float scale;
		if (isVictory_) {
			bool outerRay = (i % 4 == 0);
			if (outerRay) {
				// outer rays slowly shrink
				scale = initScale_[i] * (1.0f - 0.25f * t);
			} else {
				// inner floaters breathe (slight pulse) while fading
				float pulse = 0.08f * std::sin(counter_ * (5.0f + 0.2f * (i % 9)));
				scale = initScale_[i] * (1.0f - 0.15f * t) * (1.0f + pulse);
			}
		} else {
			scale = initScale_[i] * (1.0f + 0.6f * (1.0f - t));
		}
		worldTransforms_[i].scale_ = {scale, scale, scale};

		// fade out
		if (isVictory_) {
			// inner floaters: alpha pulse; outer rays: steady fade
			bool outerRay = (i % 4 == 0);
			if (outerRay) {
				colorVals_[i].w = std::clamp(1.0f - t, 0.0f, 1.0f);
			} else {
				float pulseA = 0.12f * std::sin(counter_ * (6.0f + 0.25f * (i % 11)));
				colorVals_[i].w = std::clamp((1.0f - t) + pulseA, 0.0f, 1.0f);
			}
		} else {
			colorVals_[i].w = std::clamp(1.0f - t, 0.0f, 1.0f);
		}
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
