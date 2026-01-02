#pragma once

#include "KamataEngine.h"
#include "AABB.h"

class Ice {
public:
    Ice() = default;
    ~Ice();
    Ice(const KamataEngine::Vector3& pos) : position_(pos) {}

    void Initialize();
    void Update(float delta);
    void Draw(KamataEngine::Camera* camera);

    void SetPosition(const KamataEngine::Vector3& pos) { position_ = pos; worldTransform_.translation_ = pos; }
    const KamataEngine::Vector3& GetPosition() const { return position_; }

    bool HasModel() const { return model_ != nullptr; }

    // Ice の軸整列AABBを取得する
    AABB GetAABB() const;

private:
    KamataEngine::Vector3 position_ = {0,0,0};
    int frame_ = 0; // アニメーションフレーム（必要なら使用）

    KamataEngine::Model* model_ = nullptr;
    bool ownsModel_ = false;
    KamataEngine::WorldTransform worldTransform_;
};