#include "Ice.h"

#include "KamataEngine.h"
#include"MathUtl.h"

using namespace KamataEngine;

Ice::~Ice() {
    if (ownsModel_ && model_) {
        delete model_;
        model_ = nullptr;
    }
}

void Ice::Initialize() {
    // Ice.obj を読み込み（Spike と同様のモデルロードスタイルに合わせる）
    model_ = Model::CreateFromOBJ("Ice", true);
    ownsModel_ = true;

    worldTransform_.Initialize();
    worldTransform_.translation_ = position_;
    worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};
}

void Ice::Update(float delta) {
    (void)delta;
    frame_++;

    // 必要に応じてアニメーションやエフェクトを追加可能
    worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
    worldTransform_.TransferMatrix();
}

void Ice::Draw(Camera* camera) {
    if (!model_) return;
    model_->Draw(worldTransform_, *camera);
}

AABB Ice::GetAABB() const {
    // Spike と同程度のサイズを仮定してAABBを作成
    const float w = 2.0f; // tile width
    const float h = 2.0f; // tile height
    Vector3 half = {w * 0.5f, h * 0.5f, 1.0f};
    Vector3 center = worldTransform_.translation_;

    AABB aabb;
    aabb.min = {center.x - half.x, center.y - half.y, center.z - half.z};
    aabb.max = {center.x + half.x, center.y + half.y, center.z + half.z};
    return aabb;
}