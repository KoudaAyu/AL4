#include "Goal.h"

#include "KamataEngine.h"
#include "MathUtl.h"

using namespace KamataEngine;

void Goal::Initialize() {
    model_ = KamataEngine::Model::CreateFromOBJ("Goal", true);
    if (model_) {
        ownsModel_ = true;
        DebugText::GetInstance()->ConsolePrintf("Goal: model 'Goal' loaded\n");
    } else {
        DebugText::GetInstance()->ConsolePrintf("Goal: failed to load model 'Goal', using fallback model\n");
        model_ = KamataEngine::Model::Create();
        if (model_) {
            ownsModel_ = true;
            DebugText::GetInstance()->ConsolePrintf("Goal: fallback model created\n");
        }
    }

    worldTransform_.Initialize();
    worldTransform_.translation_ = position_;
    worldTransform_.translation_.z = -0.5f;
    worldTransform_.rotation_ = {0, 0, 0};
    worldTransform_.scale_ = {0.9f, 0.9f, 0.9f};

    worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
    worldTransform_.TransferMatrix();
}

Goal::~Goal() {
    if (ownsModel_ && model_) {
        delete model_;
        model_ = nullptr;
    }
}

void Goal::Update(float /*delta*/) {
    worldTransform_.translation_ = position_;
    worldTransform_.translation_.z = -0.5f;
    worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
    worldTransform_.TransferMatrix();
}

void Goal::Draw(KamataEngine::Camera* camera) {
    if (!model_ || !camera) return;
    worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
    worldTransform_.TransferMatrix();
    model_->Draw(worldTransform_, *camera);
}

AABB Goal::GetAABB() const {
    static constexpr float kSize = 0.9f * 2.0f;
    Vector3 center = position_;
    Vector3 half = {kSize * 0.5f, kSize * 0.5f, kSize * 0.5f};

    AABB aabb;
    aabb.min = {center.x - half.x, center.y - half.y, center.z - half.z};
    aabb.max = {center.x + half.x, center.y + half.y, center.z + half.z};
    return aabb;
}
