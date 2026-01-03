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

    // Camera used to render the player in the select scene
    KamataEngine::Camera camera_;

    // Player instance to visualize and control
    class Player* player_ = nullptr;

    // Map chip field so player can interact with tiles (optional)
    class MapChipField* mapChipField_ = nullptr;
};
