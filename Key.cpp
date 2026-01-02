#include "Key.h"

#include "KamataEngine.h"
#include "MathUtl.h"

using namespace KamataEngine;

void Key::Initialize() {
    frame_ = 0;

    // Load key model
    model_ = KamataEngine::Model::CreateFromOBJ("Key", true);
    ownsModel_ = (model_ != nullptr);

    worldTransform_.Initialize();
    worldTransform_.translation_ = position_;
    worldTransform_.translation_.z = 0.0f;
    worldTransform_.rotation_ = {0, 0, 0};
    worldTransform_.scale_ = {0.6f, 0.6f, 0.6f};

    worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
    worldTransform_.TransferMatrix();
}

Key::~Key() {
    if (ownsModel_ && model_) {
        delete model_;
        model_ = nullptr;
    }
}

void Key::Update(float delta) {
    const float frameTime = 0.15f;
    static float acc = 0.0f;
    acc += delta;
    if (acc >= frameTime) {
        acc = 0.0f;
        frame_ = (frame_ + 1) % 4;
    }

    worldTransform_.translation_ = position_;
    worldTransform_.translation_.z = 0.0f;

    worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
    worldTransform_.TransferMatrix();
}

void Key::Draw(KamataEngine::Camera* camera) {
    if (!model_) return;
    if (!camera) return;

    worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
    worldTransform_.TransferMatrix();

    model_->Draw(worldTransform_, *camera);
}

AABB Key::GetAABB() const {
    static constexpr float kWidth = 0.6f * 2.0f;
    static constexpr float kHeight = 0.6f * 2.0f;
    static constexpr float kDepth = 0.6f * 2.0f;

    Vector3 center = position_;
    Vector3 half = {kWidth * 0.5f, kHeight * 0.5f, kDepth * 0.5f};

    AABB aabb;
    aabb.min = {center.x - half.x, center.y - half.y, center.z - half.z};
    aabb.max = {center.x + half.x, center.y + half.y, center.z + half.z};
    return aabb;
}
