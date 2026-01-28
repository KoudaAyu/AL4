#pragma once

#include "KamataEngine.h"
#include "Fade.h"
#include <2d/Sprite.h>

class GameOverScene {
public:
    enum class Phase {
        kFadeIn,
        kMain,
        kFadeOut,
    };
    enum class Result {
        kNone,
        kRetryGame,    // MorePlay
        kBackSelect,   // BackSelect
        kBackTitle,    // BackTitle
    };

    GameOverScene() = default;
    ~GameOverScene();

    void Initialize();
    void Update();
    void Draw();

    bool IsFinished() const { return finished_; }
    Result GetResult() const { return result_; }

private:
    bool finished_ = false;
    Fade* fade_ = nullptr;

    const int32_t kWindowWidth = 1280;
    const int32_t kWindowHeight = 720;

    // Full-screen background for game over
    uint32_t backgroundTextureHandle_ = 0u;
    KamataEngine::Sprite* backgroundSprite_ = nullptr;

    // Stacked menu sprites (top/mid/bottom)
    uint32_t gameOverTextureHandle_ = 0u; // middle
    KamataEngine::Sprite* gameOverSprite_ = nullptr;
    uint32_t optionLeftTextureHandle_ = 0u;   // top
    KamataEngine::Sprite* optionLeftSprite_ = nullptr;
    uint32_t optionRightTextureHandle_ = 0u;  // bottom
    KamataEngine::Sprite* optionRightSprite_ = nullptr;

    uint32_t gameOverHandle = 0u;
	KamataEngine::Sprite* gameOverSprite = nullptr;

    // Selection index: 0=top, 1=middle, 2=bottom
    int selectedIndex_ = 0;
    Result result_ = Result::kNone;

    Phase phase_ = Phase::kFadeIn;

    uint32_t seDecisionDataHandle_ = 0u;


};