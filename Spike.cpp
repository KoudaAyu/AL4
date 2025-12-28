#include "Spike.h"

#include "KamataEngine.h"
#include "MathUtl.h"

using namespace KamataEngine;

void Spike::Initialize() {
    frame_ = 0;
    
    model_ = KamataEngine::Model::CreateFromOBJ("thorn", true);
    if (model_) {
        ownsModel_ = true;
        DebugText::GetInstance()->ConsolePrintf("Spike: model 'thorn' loaded\n");
    } else {
        DebugText::GetInstance()->ConsolePrintf("Spike: failed to load model 'thorn', using fallback model\n");
      
        model_ = KamataEngine::Model::Create();
        if (model_) {
            ownsModel_ = true;
            DebugText::GetInstance()->ConsolePrintf("Spike: fallback model created\n");
        }
    }

    worldTransform_.Initialize();
    worldTransform_.translation_ = position_;
   
    worldTransform_.translation_.z = -0.5f;
    worldTransform_.rotation_ = {0, 0, 0};
  
    worldTransform_.scale_ = {0.8f, 0.8f, 0.8f};

 
    worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
    worldTransform_.TransferMatrix();
}

Spike::~Spike() {
    if (ownsModel_ && model_) {
        delete model_;
        model_ = nullptr;
    }
}

void Spike::Update(float delta) {
    // とりあえずフレーム更新だけ（将来的にアニメーションを入れる）
    const float frameTime = 0.15f;
    static float acc = 0.0f;
    acc += delta;
    if (acc >= frameTime) {
        acc = 0.0f;
        frame_ = (frame_ + 1) % 4; // 4フレーム分のアニメ用
    }

    // 位置を反映
    worldTransform_.translation_ = position_;
  
    worldTransform_.translation_.z = -0.5f;

   
    worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
    worldTransform_.TransferMatrix();
}

void Spike::Draw(KamataEngine::Camera* camera) {
    if (!model_) return;
    if (!camera) return;

  
    worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
    worldTransform_.TransferMatrix();

    model_->Draw(worldTransform_, *camera);
}

AABB Spike::GetAABB() const {
  
    static constexpr float kWidth = 0.8f * 2.0f;
    static constexpr float kHeight = 0.8f * 2.0f;
    static constexpr float kDepth = 0.8f * 2.0f;

    Vector3 center = position_;
    Vector3 half = {kWidth * 0.5f, kHeight * 0.5f, kDepth * 0.5f};

    AABB aabb;
    aabb.min = {center.x - half.x, center.y - half.y, center.z - half.z};
    aabb.max = {center.x + half.x, center.y + half.y, center.z + half.z};
    return aabb;
}
