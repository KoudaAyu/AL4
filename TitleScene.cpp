#include "TitleScene.h"

using namespace KamataEngine;

TitleScene::~TitleScene() { delete fade_; }

void TitleScene::Initialize() {

	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, 3.0f);
}

void TitleScene::Update() {

	switch (phase_)
	{
	case Phase::kFadeIn:
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
			if (fade_->IsFinished()) {
			finished_ = true;
		    }

			break;
	}

	

	fade_->Update();
}

void TitleScene::Draw() {

	fade_->Draw();

}
