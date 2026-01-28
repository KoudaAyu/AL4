#pragma once

#include "KamataEngine.h"
#include "Fade.h"

class GameClearScene {
public:
    enum class Phase {
        kFadeIn,
        kMain,
        kFadeOut,
    };

    GameClearScene() = default;
    ~GameClearScene();

    void Initialize();
    void Update();
    void Draw();

    bool IsFinished() const { return finished_; }

private:
    bool finished_ = false;
    Fade* fade_ = nullptr;

    Phase phase_ = Phase::kFadeIn;
};
