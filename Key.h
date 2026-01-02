#pragma once

#include "KamataEngine.h"
#include "AABB.h"

class Player; // forward

class Key {
public:
    Key() = default;
    ~Key();

    void Initialize();
    void Update(float delta);
    void Draw(KamataEngine::Camera* camera);

    void SetPosition(const KamataEngine::Vector3& pos) { position_ = pos; worldTransform_.translation_ = pos; }
    const KamataEngine::Vector3& GetPosition() const { return position_; }

    AABB GetAABB() const;

    bool HasModel() const { return model_ != nullptr; }

    // Play pickup sound
    void PlayGetSound();

    // Called when player touches the key to start pickup animation
    void OnPicked(Player* player);

    bool IsPicked() const;
    bool IsCollected() const;

private:
    KamataEngine::Vector3 position_{0.0f, 0.0f, 0.0f};
    int frame_ = 0; // animation frame (kept for consistency with Spike)

    KamataEngine::Model* model_ = nullptr;
    bool ownsModel_ = false;

    KamataEngine::WorldTransform worldTransform_{};

    // sound
    uint32_t soundDataHandle = KamataEngine::Audio::GetInstance()->LoadWave("Audio/SE/Key_Get.wav");

    // pickup state
    enum class State {
        kIdle,
        kRotating,
        kAttracting,
        kCollected,
    } state_ = State::kIdle;

    Player* targetPlayer_ = nullptr; // player to attract to
    float stateTimer_ = 0.0f; // generic timer for state transitions

    // tuning
    static inline constexpr float kRotateDuration = 0.6f; // seconds to rotate before attracting
    static inline constexpr float kRotateSpeed = 6.2831853f; // radians per second (1 rev/s)
    static inline constexpr float kAttractSpeed = 6.0f; // base units per second while attracted
    static inline constexpr float kCollectDistance = 0.25f; // distance to player to consider collected

    // distance-based tuning: if player is farther than this, attraction speeds up
    static inline constexpr float kNearDistanceThreshold = 1.0f; // within this distance use base speed
    static inline constexpr float kDistanceSpeedFactor = 1.0f;   // multiplier per unit beyond threshold
    static inline constexpr float kMaxAttractMultiplier = 4.0f; // cap on speed multiplier

    // if player is farther than this when rotating, skip rotation and start attracting sooner
    static inline constexpr float kRotateSkipDistance = 3.0f;

    bool collected_ = false; // true when finally consumed by player

    // original scale stored for scaling during attraction
    KamataEngine::Vector3 initialScale_{0.6f, 0.6f, 0.6f};
};
