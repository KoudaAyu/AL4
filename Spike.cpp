#include "Spike.h"

#include "KamataEngine.h"
#include "MathUtl.h"

using namespace KamataEngine;

void Spike::Initialize() {
    frame_ = 0;
    // モデルをthorn.objから読み込む
    model_ = KamataEngine::Model::CreateFromOBJ("thorn", true);
    if (model_) {
        ownsModel_ = true;
        DebugText::GetInstance()->ConsolePrintf("Spike: model 'thorn' loaded\n");
    } else {
        DebugText::GetInstance()->ConsolePrintf("Spike: failed to load model 'thorn', using fallback model\n");
        // Fallback: create a simple default model so something is visible
        model_ = KamataEngine::Model::Create();
        if (model_) {
            ownsModel_ = true;
            DebugText::GetInstance()->ConsolePrintf("Spike: fallback model created\n");
        }
    }

    worldTransform_.Initialize();
    worldTransform_.translation_ = position_;
    // Slightly bring spike forward so it is not occluded by block geometry
    worldTransform_.translation_.z = -0.5f;
    worldTransform_.rotation_ = {0, 0, 0};
    // Make it reasonably visible by default
    worldTransform_.scale_ = {0.8f, 0.8f, 0.8f};

    // Update initial matrix
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
    // keep z offset
    worldTransform_.translation_.z = -0.5f;

    // Update matrix for drawing
    worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
    worldTransform_.TransferMatrix();
}

void Spike::Draw(KamataEngine::Camera* camera) {
    if (!model_) return;
    if (!camera) return;

    // Ensure world matrix is up-to-date
    worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
    worldTransform_.TransferMatrix();

    model_->Draw(worldTransform_, *camera);
}
