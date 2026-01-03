#include "Ladder.h"

#include "KamataEngine.h"
#include "MathUtl.h"

using namespace KamataEngine;

void Ladder::Initialize() {
  
    model_ = KamataEngine::Model::CreateFromOBJ("Ladder", true);
    ownsModel_ = (model_ != nullptr);

    worldTransform_.Initialize();
    worldTransform_.translation_ = position_;
    worldTransform_.translation_.z = 0.0f;
    worldTransform_.rotation_ = {0, 0, 0};
    worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};

    worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
    worldTransform_.TransferMatrix();
}

Ladder::~Ladder() {
    if (ownsModel_ && model_) {
        delete model_;
        model_ = nullptr;
    }
}

void Ladder::Update(float delta) {
    (void)delta;
    // Ladder はアニメーションしないので位置のみ更新
    worldTransform_.translation_ = position_;
    worldTransform_.translation_.z = 0.0f;

    worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
    worldTransform_.TransferMatrix();
}

void Ladder::Draw(KamataEngine::Camera* camera) {
    if (!model_) return;
    if (!camera) return;

    worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
    worldTransform_.TransferMatrix();

    model_->Draw(worldTransform_, *camera);
}

AABB Ladder::GetAABB() const {
    // Ladder は縦長の当たり判定にする
    static constexpr float kWidth = 1.0f * 2.0f; // 実際の幅に合わせて調整
    static constexpr float kHeight = 2.0f * 2.0f; // 高さをブロック2つ分に設定（必要なら調整）
    static constexpr float kDepth = 1.0f * 2.0f;

    Vector3 center = position_;
    Vector3 half = {kWidth * 0.5f, kHeight * 0.5f, kDepth * 0.5f};

    AABB aabb;
    aabb.min = {center.x - half.x, center.y - half.y, center.z - half.z};
    aabb.max = {center.x + half.x, center.y + half.y, center.z + half.z};
    return aabb;
}
