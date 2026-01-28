#pragma once
#include "KamataEngine.h"

#include <array>
#include <random>
#include <numbers>

class EnemyDeathParticle {
public:
    EnemyDeathParticle();
    ~EnemyDeathParticle();

    // Initialize effect with model, camera and world position
    void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& pos);

    // Update per-frame (called from scene)
    void Update();

    // Draw particles
    void Draw();

    // Has the effect finished?
    bool IsFinished() const { return isFinish_; }

    // Reset/stop immediately
    void Reset();

private:
    static inline const uint32_t kNumParticles = 12; // fewer particles for a subtler effect
    static inline const float kDuration = 1.0f;        // shorter, more subtle
    static inline const float kBaseSpeed = 0.2f;       // slower movement

    bool isFinish_ = false;
    float counter_ = 0.0f; // elapsed seconds

    std::array<KamataEngine::WorldTransform, kNumParticles> worldTransforms_{};
    std::array<KamataEngine::Vector3, kNumParticles> velocities_{};
    std::array<float, kNumParticles> angularVel_{};
    std::array<float, kNumParticles> initScale_{};
    std::array<KamataEngine::ObjectColor, kNumParticles> objectColors_{};
    std::array<KamataEngine::Vector4, kNumParticles> colorVals_{};

    // gentle per-particle horizontal wobble to make effect feel airy
    std::array<float, kNumParticles> wobblePhase_{};
    std::array<float, kNumParticles> wobbleSpeed_{};

    KamataEngine::Model* model_ = nullptr;
    KamataEngine::Camera* camera_ = nullptr;

    // base color for enemy death: softer, desaturated bluish tone
    KamataEngine::Vector4 baseColor_ = {0.6f, 0.7f, 0.95f, 1.0f};

    std::mt19937 rnd_;
};
