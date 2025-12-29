#pragma once

#include "KamataEngine.h"
#include "Fade.h"

class GameOverScene {
public:
    enum class Phase {
        kFadeIn,
        kMain,
        kFadeOut,
    };

    GameOverScene() = default;
    ~GameOverScene();

    void Initialize();
    void Update();
    void Draw();

    bool IsFinished() const { return finished_; }

private:
    bool finished_ = false;
    Fade* fade_ = nullptr;

    Phase phase_ = Phase::kFadeIn;
};