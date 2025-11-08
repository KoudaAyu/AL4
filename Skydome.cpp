#include "Skydome.h"

using namespace KamataEngine;

void Skydome::Initialize() {
	worldTransform_.Initialize();
	// モデル生成（OBJ: Resources/Model/skydome/skydome.obj を想定）
	model_ = Model::CreateFromOBJ("skydome", true);
	// 大きくして、原点中心に配置
	worldTransform_.scale_ = { 100.0f, 100.0f, 100.0f };
}

void Skydome::Update()
{}

void Skydome::Draw() {
	if (!model_ || !camera_) { return; }
	model_->Draw(worldTransform_, *camera_);
}