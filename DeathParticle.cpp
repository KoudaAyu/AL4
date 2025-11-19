#include "DeathParticle.h"


#include<algorithm>
#include"MathUtl.h"

using namespace KamataEngine;

DeathParticle::DeathParticle() {}

DeathParticle::~DeathParticle() {}

void DeathParticle::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& pos) {
#ifdef _DEBUG
	assert(model);
	#endif
	// 引数として受け取ったデータをメンバ関数に記録
	model_ = model;
	/*textureHandle_ = textureHandle;*/
	camera_ = camera;

	objectColor_.Initialize();
	color_ = {1.0f, 1.0f, 1.0f, 1.0f};

	// 各パーティクルのワールド変換の初期化
	
	for (WorldTransform& worldTransform : worldTransforms_)
	{
		worldTransform.Initialize();
		worldTransform.translation_ = pos;
	}
}


void DeathParticle::Update() {

	if (isFinish_)
	{
		return;
	}

	emitOctagonalParticles();

	counter_ += 1.0f / 60.0f;
	if (counter_ >= kDuration) {
		counter_ = kDuration;
		isFinish_ = true;
	}

	
	float t = counter_ / kDuration; 
	color_.w = std::clamp(1.0f - t, 0.0f, 1.0f);
	objectColor_.SetColor(color_);
	
	for (auto& wt : worldTransforms_) {

		wt.matWorld_ = MakeAffineMatrix(wt.scale_, wt.rotation_, wt.translation_);
		wt.TransferMatrix();
	}

}


void DeathParticle::Draw() { 

	if (isFinish_)
	{
		return;
	}
	for (auto& wt : worldTransforms_) {
		model_->Draw(wt, *camera_,&objectColor_);
	}
}

void DeathParticle::emitOctagonalParticles() {

	for (uint32_t i = 0; i < kNumParticles; ++i) {
		// 速度
		Vector3 velocity = {kSpeed, 0.0f, 0.0f};
		// 回転角
		float angle = kAngleUnit * i;
		// Z軸回りの回転行列を作成
		Matrix4x4 rotMat = MakeRotateZMatrix(angle);

		velocity = Transform(velocity, rotMat);

		worldTransforms_[i].translation_ += velocity;
	}

}
