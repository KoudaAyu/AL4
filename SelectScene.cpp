#include "SelectScene.h"
#include "KeyInput.h"
#include <2d/Sprite.h>

#include "Player.h"
#include "MapChipField.h"
#include "CameraController.h"

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
    if (blockModel_) {
        delete blockModel_;
        blockModel_ = nullptr;
    }

    if (cameraController_) {
        delete cameraController_;
        cameraController_ = nullptr;
    }

    for (auto& row : worldTransformBlocks_) {
        for (WorldTransform* wt : row) {
            delete wt;
        }
    }
    worldTransformBlocks_.clear();
}

void SelectScene::Initialize() {
  
    camera_.Initialize();
    camera_.translation_ = {0.0f, 0.0f, -50.0f};
    camera_.UpdateMatrix();
    camera_.TransferMatrix();

 
    cameraController_ = new CameraController();

   
    mapChipField_ = new MapChipField();
   
    mapChipField_->LoadMapChipCsv("Resources/Map/SelectScene/SelectScene.csv");

  
    blockModel_ = Model::CreateFromOBJ("Block");

  
    if (mapChipField_) {
        uint32_t vh = mapChipField_->GetNumBlockVertical();
        uint32_t wh = mapChipField_->GetNumBlockHorizontal();
        worldTransformBlocks_.assign(vh, std::vector<WorldTransform*>(wh, nullptr));
        for (uint32_t y = 0; y < vh; ++y) {
            for (uint32_t x = 0; x < wh; ++x) {
                MapChipType t = mapChipField_->GetMapChipTypeByIndex(x, y);
                if (t == MapChipType::kBlock || t == MapChipType::kIce) {
                    WorldTransform* wt = new WorldTransform();
                    wt->Initialize();
                    wt->translation_ = mapChipField_->GetMapChipPositionByIndex(x, y);
                    worldTransformBlocks_[y][x] = wt;
                }
            }
        }
    }

 
    player_ = new Player();
    Vector3 startPos = {4.0f, 4.0f, 0.0f};
  
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

   
    if (cameraController_) {
        if (mapChipField_) {
            cameraController_->SetMovableArea(mapChipField_->GetMovableArea());
        } else {
            cameraController_->SetMovableArea({-50.0f, 50.0f, 50.0f, -50.0f});
        }
        cameraController_->Initialize(&camera_);
        cameraController_->SetTarget(player_);
        player_->SetCameraController(cameraController_);
        cameraController_->Reset();
    }
}

void SelectScene::Update() {
   
    bool spacePressed = Input::GetInstance()->PushKey(DIK_SPACE);
    bool shiftPressed = Input::GetInstance()->PushKey(DIK_LSHIFT) || Input::GetInstance()->PushKey(DIK_RSHIFT);

    if ((spacePressed && shiftPressed) || KeyInput::GetInstance()->TriggerPadButton(KeyInput::XINPUT_BUTTON_A)) {
        finished_ = true;
        return;
    }

   
    if (cameraController_) {
        cameraController_->Update();
    } else {
        camera_.UpdateMatrix();
    }

   
    if (player_) {
        player_->Update();
    }
}

void SelectScene::Draw() {
    Model::PreDraw();

   
    if (!worldTransformBlocks_.empty()) {
        uint32_t vh = static_cast<uint32_t>(worldTransformBlocks_.size());
        uint32_t wh = static_cast<uint32_t>(worldTransformBlocks_[0].size());
        for (uint32_t y = 0; y < vh; ++y) {
            for (uint32_t x = 0; x < wh; ++x) {
                WorldTransform* wt = worldTransformBlocks_[y][x];
                if (!wt) continue;
              
                wt->matWorld_ = MakeAffineMatrix(wt->scale_, wt->rotation_, wt->translation_);
                if (wt->parent_) wt->matWorld_ = Multiply(wt->parent_->matWorld_, wt->matWorld_);
                wt->TransferMatrix();

                if (blockModel_) {
                    blockModel_->Draw(*wt, camera_);
                }
            }
        }
    }

    if (player_) player_->Draw();

    Model::PostDraw();
}
