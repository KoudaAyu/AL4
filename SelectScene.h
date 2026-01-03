#pragma once

#include "KamataEngine.h"
#include <vector>

class CameraController; // forward

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

    // Models used to draw blocks
    KamataEngine::Model* blockModel_ = nullptr;

    // world transforms for blocks (rows x cols)
    std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;

    // Camera controller to follow the player similar to GameScene
    CameraController* cameraController_ = nullptr;
};
