#include "TitleScene.h"
#include "MathUtl.h"
#include <cassert>
#include <numbers>

using namespace KamataEngine;

TitleScene::~TitleScene() {
	delete fade_;
	if (model_) {
		delete model_;
		model_ = nullptr;
	}
}

void TitleScene::Initialize() {

	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, 3.0f);

	// Create title model from OBJ placed in Resources/Models/Title.obj (name: "title")
	model_ = Model::CreateFromOBJ("title", true);
	// Ensure model was created in debug builds
#ifdef _DEBUG
	assert(model_);
#endif

	// Initialize camera for title scene
	camera_.Initialize();
	// Place camera so title model is visible: use default farther distance
	camera_.translation_ = {0.0f, 0.0f, -50.0f};
	camera_.UpdateMatrix();
	camera_.TransferMatrix();

	// World transform for title model
	worldTransform_.Initialize();
	worldTransform_.translation_ = {0.0f, 0.0f, 0.0f};
	// Rotate model so it faces the camera (fix model that is pointing upward)
	worldTransform_.rotation_.x = std::numbers::pi_v<float> / 2.0f; // flipped from -pi/2 to +pi/2 to correct upside-down text
	// Increase scale so model is likely visible
	worldTransform_.scale_ = {5.0f, 5.0f, 5.0f};

}

void TitleScene::Update() {

	switch (phase_)
	{
	case Phase::kFadeIn:
		// advance fade once per frame
		fade_->Update();

		if (fade_->IsFinished()) {
			phase_ = Phase::kMain;
		}

		break;

		case Phase::kMain:
		if (Input::GetInstance()->PushKey(DIK_SPACE)) {
			phase_ = Phase::kFadeOut;
			fade_->Start(Fade::Status::FadeOut, 3.0f);
		}

		break;

		case Phase::kFadeOut:
			// advance fade once per frame so it can finish
			fade_->Update();
			if (fade_->IsFinished()) {
			finished_ = true;
		    }

			break;
	}

	// rotate title slowly around Y
	worldTransform_.rotation_.y += 0.5f * (1.0f / 60.0f);
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();

}

void TitleScene::Draw() {

	Model::PreDraw();
	if (model_) {
		model_->Draw(worldTransform_, camera_);
	}
	Model::PostDraw();

	fade_->Draw();

}
