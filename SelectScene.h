#pragma once

#include "KamataEngine.h"

class SelectScene {
public:
    SelectScene() = default;
    ~SelectScene();

    void Initialize();
    void Update();
    void Draw();

    bool IsFinished() const { return finished_; }

private:
    bool finished_ = false;
};
