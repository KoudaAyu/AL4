#include "SelectScene.h"
#include "KeyInput.h"
#include <2d/Sprite.h>

#include "Player.h"
#include "MapChipField.h"

using namespace KamataEngine;

SelectScene::~SelectScene() {
    if (player_) {
        delete player_;
        player_ = nullptr;
    }
    if (mapChipField_) {
        delete mapChipField_;
        mapChipField_ = nullptr;
    }
}

void SelectScene::Initialize() {
    // Initialize camera for a simple 2D-ish view
    camera_.Initialize();
    camera_.translation_ = {0.0f, 0.0f, -50.0f};
    camera_.UpdateMatrix();
    camera_.TransferMatrix();

    // Load a small map so player can stand and move
    mapChipField_ = new MapChipField();
    // Attempt to load a default map; fall back silently if missing
    mapChipField_->LoadMapChipCsv("Resources/Debug/Map/Block.csv");

    // Create player at a reasonable position
    player_ = new Player();
    Vector3 startPos = {4.0f, 4.0f, 0.0f};
    // try to find spawn in the map if present
    if (mapChipField_) {
        uint32_t vh = mapChipField_->GetNumBlockVertical();
        uint32_t wh = mapChipField_->GetNumBlockHorizontal();
        bool found = false;
        for (uint32_t y = 0; y < vh && !found; ++y) {
            for (uint32_t x = 0; x < wh; ++x) {
                if (mapChipField_->GetMapChipTypeByIndex(x, y) == MapChipType::kReserved2) {
                    startPos = mapChipField_->GetMapChipPositionByIndex(x, y);
                    found = true;
                    break;
                }
            }
        }
    }

    player_->Initialize(&camera_, startPos);
    player_->SetMapChipField(mapChipField_);
}

void SelectScene::Update() {
    // Temporarily require Left Shift + Space to proceed so plain Space won't advance
    bool spacePressed = Input::GetInstance()->PushKey(DIK_SPACE);
    bool shiftPressed = Input::GetInstance()->PushKey(DIK_LSHIFT) || Input::GetInstance()->PushKey(DIK_RSHIFT);

    if ((spacePressed && shiftPressed) || KeyInput::GetInstance()->TriggerPadButton(KeyInput::XINPUT_BUTTON_A)) {
        finished_ = true;
        return;
    }

    // Update camera if you want (keep static for now)
    camera_.UpdateMatrix();

    // Allow the player to receive input and update
    if (player_) {
        player_->Update();
    }
}

void SelectScene::Draw() {
    Model::PreDraw();

    if (player_) player_->Draw();

    Model::PostDraw();
}
