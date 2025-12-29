#include "GameClearScene.h"
#include "KeyInput.h"
#include "KamataEngine.h"

using namespace KamataEngine;

GameClearScene::~GameClearScene() {
    delete fade_;
    fade_ = nullptr;
}

void GameClearScene::Initialize() {
    fade_ = new Fade();
    fade_->Initialize();
    fade_->Start(Fade::Status::FadeIn, 2.0f);
    phase_ = Phase::kFadeIn;
    finished_ = false;
}

void GameClearScene::Update() {
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

void GameClearScene::Draw() {
    
    if (fade_) fade_->Draw();
}
