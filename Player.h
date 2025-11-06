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

    // 追加: 使用カメラの差し替え
    void SetCamera(KamataEngine::Camera* camera) { camera_ = camera; }

public:
    const KamataEngine::WorldTransform& GetWorldTransform() const { return worldTransform_; }
    const KamataEngine::Vector3& GetPosition() const { return worldTransform_.translation_; }
    const KamataEngine::Vector3& GetVelocity() const { return velocity_; }

private:
    KamataEngine::WorldTransform worldTransform_;
    KamataEngine::Model* model_ = nullptr;
    KamataEngine::Camera* camera_ = nullptr;
    uint32_t textureHandle_ = 0u;
    KamataEngine::Vector3 velocity_ = {};
    LRDirection lrDirection_ = LRDirection::Right;
    static inline const float kAcceleration = 0.05f;
    static inline const float kAttenuation = 0.1f;
    static inline const float kLimitSpeed = 2.0f;
    float turnFirstRotationY_ = 0.0f;
    float turnTimer_ = 0.0f;
    static inline const float kTimeTurn = 0.3f;
    bool isJump_ = false;
    float jumpVelocity_ = 0.0f;
    static inline float kGravity = 0.2f;
    static inline float kJumpVelocity = 2.0f;
    int jumpCount_ = 0;
    static inline const int kMaxJumpCount = 2;
};

