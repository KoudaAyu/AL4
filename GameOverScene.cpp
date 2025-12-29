#include "GameOverScene.h"
#include "KeyInput.h"
#include "KamataEngine.h"

using namespace KamataEngine;

GameOverScene::~GameOverScene() {
    delete fade_;
    fade_ = nullptr;
}

void GameOverScene::Initialize() {
    fade_ = new Fade();
    fade_->Initialize();
    fade_->Start(Fade::Status::FadeIn, 2.0f);
    phase_ = Phase::kFadeIn;
    finished_ = false;
}

void GameOverScene::Update() {
    switch (phase_) {
    case Phase::kFadeIn:
        fade_->Update();
        if (fade_->IsFinished()) {
            phase_ = Phase::kMain;
        }
        break;
    case Phase::kMain:
       
        if (Input::GetInstance()->PushKey(DIK_SPACE) || KeyInput::GetInstance()->TriggerPadButton(KeyInput::XINPUT_BUTTON_A)) {
            phase_ = Phase::kFadeOut;
            fade_->Start(Fade::Status::FadeOut, 2.0f);
        }
        break;
    case Phase::kFadeOut:
        fade_->Update();
        if (fade_->IsFinished()) {
            finished_ = true;
        }
        break;
    }
}

void GameOverScene::Draw() {
   
    if (fade_) fade_->Draw();
}
