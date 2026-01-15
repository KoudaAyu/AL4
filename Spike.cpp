#include "Spike.h"

#include "KamataEngine.h"
#include "MathUtl.h"

using namespace KamataEngine;

void Spike::Initialize() {
    frame_ = 0;

    // Load fixed spike model
    model_ = KamataEngine::Model::CreateFromOBJ("thorn", true);
    ownsModel_ = (model_ != nullptr);

    worldTransform_.Initialize();
    worldTransform_.translation_ = position_;
    worldTransform_.translation_.z = 0.0f;
    worldTransform_.rotation_ = {0, 0, 0};
    worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};

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

void Spike::Draw(KamataEngine::Camera* camera) {
    if (!model_) return;
    if (!camera) return;

    worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
    worldTransform_.TransferMatrix();

    model_->Draw(worldTransform_, *camera);
}

AABB Spike::GetAABB() const {
	// 横幅を1マス(1.0f)よりわずかに小さく（0.9fなど）する
	// これで隣のブロックとの境界で「壁」として衝突するリスクをゼロにします
	static constexpr float kWidth = 0.9f;
	static constexpr float kDepth = 0.9f;

	// 高さは、ブロックの表面(0.8f)より少しだけ高く(0.85fなど)設定
	static constexpr float kHeightOffset = 0.85f;

	Vector3 center = position_;
	float halfW = kWidth * 0.5f;
	float halfD = kDepth * 0.5f;

	AABB aabb;
	aabb.min = {center.x - halfW, center.y - 0.8f, center.z - halfD};
	// 0.85f にすることで、トゲの上に立っている時に足先が 0.05f だけ重なり、ダメージが通ります
	aabb.max = {center.x + halfW, center.y + kHeightOffset, center.z + halfD};

	return aabb;
}