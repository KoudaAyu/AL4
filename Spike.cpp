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
	if (!model_)
		return;
	if (!camera)
		return;

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();

	model_->Draw(worldTransform_, *camera);
}

AABB Spike::GetAABB() const {
	// 横幅を1マス(1.0f)よりわずかに小さく
	static constexpr float kWidth = 0.9f;
	static constexpr float kDepth = 0.9f;

	// プレイヤーが上に立ったときにも確実に重なるように高さを少し高めに設定
	// タイル1マス分 + 余裕を持たせる
	static constexpr float kTopHeightOffset = 1.05f; // 以前の0.85fから拡張
	static constexpr float kBottomOffset = 0.4f;     // 以前の0.8fから緩めて、横衝突の誤反応を減らす

	Vector3 center = position_;
	float halfW = kWidth * 0.5f;
	float halfD = kDepth * 0.5f;

	AABB aabb;
	aabb.min = {center.x - halfW, center.y - kBottomOffset, center.z - halfD};
	// トゲ上面を少し高めにして、プレイヤーの足元と確実に重なるようにする
	aabb.max = {center.x + halfW, center.y + kTopHeightOffset, center.z + halfD};

	return aabb;
}