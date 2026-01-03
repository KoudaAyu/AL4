#pragma once

#include "KamataEngine.h"
#include "AABB.h"

class Player;

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

  
    void PlayGetSound();

   
    void OnPicked(Player* player);

    bool IsPicked() const;
    bool IsCollected() const;

private:
    KamataEngine::Vector3 position_{0.0f, 0.0f, 0.0f};
    int frame_ = 0;

    KamataEngine::Model* model_ = nullptr;
    bool ownsModel_ = false;

    KamataEngine::WorldTransform worldTransform_{};

    uint32_t soundDataHandle = KamataEngine::Audio::GetInstance()->LoadWave("Audio/SE/Key_Get.wav");


    enum class State {
        kIdle,
        kRotating,
        kAttracting,
        kCollected,
    } state_ = State::kIdle;

    Player* targetPlayer_ = nullptr;
    float stateTimer_ = 0.0f;

    
    static inline constexpr float kRotateDuration = 0.6f; 
    static inline constexpr float kRotateSpeed = 6.2831853f; 
    static inline constexpr float kAttractSpeed = 6.0f;
    static inline constexpr float kCollectDistance = 0.25f; 

    static inline constexpr float kNearDistanceThreshold = 1.0f; 
    static inline constexpr float kDistanceSpeedFactor = 1.0f;  
    static inline constexpr float kMaxAttractMultiplier = 4.0f;


    static inline constexpr float kRotateSkipDistance = 3.0f;

    bool collected_ = false; 
    
    KamataEngine::Vector3 initialScale_{0.6f, 0.6f, 0.6f};
};
