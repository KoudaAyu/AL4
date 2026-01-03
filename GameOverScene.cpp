#include "GameOverScene.h"
#include "KeyInput.h"
#include "KamataEngine.h"

using namespace KamataEngine;

GameOverScene::~GameOverScene() {
    if (gameOverSprite_) { delete gameOverSprite_; gameOverSprite_ = nullptr; }
    if (optionLeftSprite_) { delete optionLeftSprite_; optionLeftSprite_ = nullptr; }
    if (optionRightSprite_) { delete optionRightSprite_; optionRightSprite_ = nullptr; }
    delete fade_;
    fade_ = nullptr;
}

void GameOverScene::Initialize() {
    fade_ = new Fade();
    fade_->Initialize();
    fade_->Start(Fade::Status::FadeIn, 2.0f);
    phase_ = Phase::kFadeIn;
    finished_ = false;
    result_ = Result::kNone;

    // Load textures
    uint32_t texTop = TextureManager::Load("Sprite/GameOver/MorePlay.png");
    uint32_t texMid = TextureManager::Load("Sprite/GameOver/BackSelect.png");
    uint32_t texBot = TextureManager::Load("Sprite/GameOver/BackTitle.png");

    // Create centered stacked sprites
    const float centerX = static_cast<float>(kWindowWidth) * 0.5f;
    const float itemW = 400.0f;
    const float itemH = 80.0f;
    const float spacing = 25.0f;
    const float startY = static_cast<float>(kWindowHeight) * 0.5f - (itemH * 1.5f + spacing);

    if (texTop != 0u) {
        optionLeftTextureHandle_ = texTop;
        optionLeftSprite_ = KamataEngine::Sprite::Create(texTop, KamataEngine::Vector2{centerX, startY}, KamataEngine::Vector4{1,1,1,1}, KamataEngine::Vector2{0.5f, 0.5f});
        if (optionLeftSprite_) optionLeftSprite_->SetSize(KamataEngine::Vector2{itemW, itemH});
    }
    if (texMid != 0u) {
        gameOverTextureHandle_ = texMid;
        gameOverSprite_ = KamataEngine::Sprite::Create(texMid, KamataEngine::Vector2{centerX, startY + itemH + spacing}, KamataEngine::Vector4{1,1,1,1}, KamataEngine::Vector2{0.5f, 0.5f});
        if (gameOverSprite_) gameOverSprite_->SetSize(KamataEngine::Vector2{itemW, itemH});
    }
    if (texBot != 0u) {
        optionRightTextureHandle_ = texBot;
        optionRightSprite_ = KamataEngine::Sprite::Create(texBot, KamataEngine::Vector2{centerX, startY + (itemH + spacing) * 2.0f}, KamataEngine::Vector4{1,1,1,1}, KamataEngine::Vector2{0.5f, 0.5f});
        if (optionRightSprite_) optionRightSprite_->SetSize(KamataEngine::Vector2{itemW, itemH});
    }

    selectedIndex_ = 0; // start at top (MorePlay)
}

void GameOverScene::Update() {
    // Layout constants
    const float centerX = static_cast<float>(kWindowWidth) * 0.5f;
    const float itemH = 80.0f;
    const float spacing = 25.0f;
    const float startY = static_cast<float>(kWindowHeight) * 0.5f - (itemH * 1.5f + spacing);

    // Navigation: W/S or Up/Down to change selection
    bool up = Input::GetInstance()->TriggerKey(DIK_W) || Input::GetInstance()->TriggerKey(DIK_UP);
    bool down = Input::GetInstance()->TriggerKey(DIK_S) || Input::GetInstance()->TriggerKey(DIK_DOWN);
    if (up) { selectedIndex_ = (selectedIndex_ + 2) % 3; } // wrap upward
    if (down) { selectedIndex_ = (selectedIndex_ + 1) % 3; } // wrap downward

    // Apply colors: selected red, others white
    auto setColor = [&](KamataEngine::Sprite* s, int idx){
        if (!s) return;
        if (selectedIndex_ == idx) {
            s->SetColor(KamataEngine::Vector4{1,0,0,1}); // red
        } else {
            s->SetColor(KamataEngine::Vector4{1,1,1,1}); // white
        }
    };

    if (optionLeftSprite_) {
        optionLeftSprite_->SetPosition(KamataEngine::Vector2{centerX, startY});
        setColor(optionLeftSprite_, 0);
    }
    if (gameOverSprite_) {
        gameOverSprite_->SetPosition(KamataEngine::Vector2{centerX, startY + itemH + spacing});
        setColor(gameOverSprite_, 1);
    }
    if (optionRightSprite_) {
        optionRightSprite_->SetPosition(KamataEngine::Vector2{centerX, startY + (itemH + spacing) * 2.0f});
        setColor(optionRightSprite_, 2);
    }

    switch (phase_) {
    case Phase::kFadeIn:
        if (fade_) fade_->Update();
        if (fade_ && fade_->IsFinished()) { phase_ = Phase::kMain; }
        break;
    case Phase::kMain:
        // Confirm with Space or Gamepad A
        if (Input::GetInstance()->TriggerKey(DIK_SPACE) || KeyInput::GetInstance()->TriggerPadButton(KeyInput::XINPUT_BUTTON_A)) {
            // Set result based on selected index
            if (selectedIndex_ == 0) result_ = Result::kRetryGame;        // MorePlay
            else if (selectedIndex_ == 1) result_ = Result::kBackSelect;  // BackSelect -> SelectScene
            else result_ = Result::kBackTitle;                            // BackTitle

            phase_ = Phase::kFadeOut;
            if (fade_) fade_->Start(Fade::Status::FadeOut, 1.0f);
        }
        break;
    case Phase::kFadeOut:
        if (fade_) fade_->Update();
        if (fade_ && fade_->IsFinished()) { finished_ = true; }
        break;
    }
}

void GameOverScene::Draw() {
    KamataEngine::DirectXCommon* dx = KamataEngine::DirectXCommon::GetInstance();
    KamataEngine::Sprite::PreDraw(dx->GetCommandList());

    if (optionLeftSprite_) optionLeftSprite_->Draw();
    if (gameOverSprite_) gameOverSprite_->Draw();
    if (optionRightSprite_) optionRightSprite_->Draw();

    if (fade_) fade_->Draw();

    KamataEngine::Sprite::PostDraw();
}
