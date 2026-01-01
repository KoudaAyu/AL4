#pragma once

#include "KamataEngine.h"
#include "AABB.h"

class Goal {
public:
    Goal() = default;
    ~Goal();

    void Initialize();
    void Update(float delta);
    void Draw(KamataEngine::Camera* camera);

    void SetPosition(const KamataEngine::Vector3& pos) { position_ = pos; }
    const KamataEngine::Vector3& GetPosition() const { return position_; }

    AABB GetAABB() const;

    bool HasModel() const { return model_ != nullptr; }

private:
    KamataEngine::Model* model_ = nullptr;
    bool ownsModel_ = false;

    KamataEngine::WorldTransform worldTransform_{};
    KamataEngine::Vector3 position_{0.0f, 0.0f, 0.0f};
};