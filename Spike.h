#pragma once

#include "KamataEngine.h"

class Spike {
public:
    Spike() = default;
    ~Spike();
    Spike(const KamataEngine::Vector3& pos) : position_(pos) {}

    void Initialize();
    void Update(float delta);
    void Draw(KamataEngine::Camera* camera);

    void SetPosition(const KamataEngine::Vector3& pos) { position_ = pos; worldTransform_.translation_ = pos; }
    const KamataEngine::Vector3& GetPosition() const { return position_; }

    bool HasModel() const { return model_ != nullptr; }

private:
    KamataEngine::Vector3 position_ = {0,0,0};
    int frame_ = 0; // アニメーションフレーム

    // Model handling similar to Player
    KamataEngine::Model* model_ = nullptr;
    bool ownsModel_ = false;
    KamataEngine::WorldTransform worldTransform_;
};
