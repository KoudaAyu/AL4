#include "EnemyDeathParticle.h"
#include "MathUtl.h"

using namespace KamataEngine;

EnemyDeathParticle::EnemyDeathParticle() {
    std::random_device rd;
    rnd_.seed(rd());
}

EnemyDeathParticle::~EnemyDeathParticle() {}

void EnemyDeathParticle::Initialize(Model* model, Camera* camera, const Vector3& pos) {
#ifdef _DEBUG
    assert(model);
#endif
    model_ = model;
    camera_ = camera;

    std::uniform_real_distribution<float> angDist(0.0f, 2.0f * std::numbers::pi_v<float>);
    // subdued speeds
    std::uniform_real_distribution<float> speedDist(0.5f * kBaseSpeed, 1.0f * kBaseSpeed);
    std::uniform_real_distribution<float> scaleDist(0.4f, 1.0f);
    std::uniform_real_distribution<float> rotDist(-1.5f, 1.5f);
    std::uniform_real_distribution<float> jitter(-0.08f, 0.08f);
    std::uniform_real_distribution<float> wobPhase(0.0f, 2.0f * std::numbers::pi_v<float>);
    std::uniform_real_distribution<float> wobSpeed(1.0f, 3.0f);

    for (uint32_t i = 0; i < kNumParticles; ++i) {
        worldTransforms_[i].Initialize();
        worldTransforms_[i].translation_ = pos;

        const float angle = angDist(rnd_);
        const float speed = speedDist(rnd_);
        // upward-biased initial direction with smaller horizontal spread
        Vector3 dir = { std::cos(angle) * 0.6f, 0.5f + (std::uniform_real_distribution<float>(0.0f, 0.5f))(rnd_),
                        (std::uniform_real_distribution<float>(-0.15f, 0.15f))(rnd_) };
        velocities_[i] = Normalize(dir) * speed;

        angularVel_[i] = rotDist(rnd_);
        initScale_[i] = scaleDist(rnd_);
        worldTransforms_[i].scale_ = { initScale_[i], initScale_[i], initScale_[i] };

        // per-particle color variation (subtle)
        float r = std::clamp(baseColor_.x + jitter(rnd_), 0.0f, 1.0f);
        float g = std::clamp(baseColor_.y + jitter(rnd_), 0.0f, 1.0f);
        float b = std::clamp(baseColor_.z + jitter(rnd_), 0.0f, 1.0f);
        colorVals_[i] = { r, g, b, 1.0f };
        objectColors_[i].Initialize();
        objectColors_[i].SetColor(colorVals_[i]);

        // gentle wobble parameters
        wobblePhase_[i] = wobPhase(rnd_);
        wobbleSpeed_[i] = wobSpeed(rnd_);
    }

    isFinish_ = false;
    counter_ = 0.0f;
}

void EnemyDeathParticle::Update() {
    if (isFinish_) return;

    const float dt = 1.0f / 60.0f;
    counter_ += dt;
    if (counter_ >= kDuration) {
        counter_ = kDuration;
        isFinish_ = true;
    }

    const float t = counter_ / kDuration;

    for (uint32_t i = 0; i < kNumParticles; ++i) {
        // very light gravity to keep particles drifting upward slowly
        velocities_[i].y -= 0.002f;
        // slow down slightly over time
        velocities_[i] *= 0.996f;

        // apply velocity
        worldTransforms_[i].translation_ += velocities_[i];

        // add gentle horizontal wobble that decays over time
        const float wobbleAmp = 0.04f * initScale_[i] * (1.0f - t);
        const float wob = std::sin(wobblePhase_[i] + wobbleSpeed_[i] * counter_);
        worldTransforms_[i].translation_.x += wob * wobbleAmp;
        worldTransforms_[i].translation_.z += std::cos(wobblePhase_[i] + wobbleSpeed_[i] * counter_) * wobbleAmp * 0.6f;

        // slow rotation
        worldTransforms_[i].rotation_.z += angularVel_[i] * dt;

        // subtle shrink over lifetime
        const float scale = initScale_[i] * (1.0f - 0.4f * t);
        worldTransforms_[i].scale_ = { scale, scale, scale };

        // fade out smoothly
        colorVals_[i].w = std::clamp(1.0f - t, 0.0f, 1.0f);
        objectColors_[i].SetColor(colorVals_[i]);

        worldTransforms_[i].matWorld_ = MakeAffineMatrix(worldTransforms_[i].scale_, worldTransforms_[i].rotation_, worldTransforms_[i].translation_);
        worldTransforms_[i].TransferMatrix();
    }
}

void EnemyDeathParticle::Draw() {
    if (isFinish_) return;

    for (uint32_t i = 0; i < kNumParticles; ++i) {
        model_->Draw(worldTransforms_[i], *camera_, &objectColors_[i]);
    }
}

void EnemyDeathParticle::Reset() {
    isFinish_ = true;
    counter_ = kDuration;
}
