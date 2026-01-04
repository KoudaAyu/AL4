#include "SelectScene.h"
#include "KeyInput.h"
#include <2d/Sprite.h>

#include "Player.h"
#include "MapChipField.h"
#include "CameraController.h"
#include "Fade.h"

#include <algorithm>
#include <cmath>

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
    if (stageModel_) {
        delete stageModel_;
        stageModel_ = nullptr;
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

  
    for (WorldTransform* wt : stageWorldTransforms_) {
        delete wt;
    }
    stageWorldTransforms_.clear();

    if (fade_) {
        delete fade_;
        fade_ = nullptr;
    }
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
   
    stageModel_ = Model::CreateFromOBJ("Stage");

  
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

         std::vector<WorldTransform*> tmpStageWts;
        for (uint32_t y = 0; y < vh; ++y) {
            for (uint32_t x = 0; x < wh; ++x) {
                if (mapChipField_->GetMapChipTypeByIndex(x, y) == MapChipType::kStage) {
                    Vector3 pos = mapChipField_->GetMapChipPositionByIndex(x, y);
                    WorldTransform* swt = new WorldTransform();
                    swt->Initialize();
                    swt->translation_ = pos;
                    swt->scale_ = {1.0f, 1.0f, 1.0f};
                    tmpStageWts.push_back(swt);
                }
            }
        }

        std::sort(tmpStageWts.begin(), tmpStageWts.end(), [](const WorldTransform* a, const WorldTransform* b) {
            if (std::abs(a->translation_.x - b->translation_.x) > 0.0001f) return a->translation_.x < b->translation_.x;
            return a->translation_.y < b->translation_.y;
        });

      
        for (WorldTransform* swt : tmpStageWts) {
            stagePositions_.push_back(swt->translation_);
            stageWorldTransforms_.push_back(swt);
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

    // fade controller
    fade_ = new Fade();
    fade_->Initialize();
}

static float LerpF(float a, float b, float t) { return a + (b - a) * t; }
static KamataEngine::Vector3 LerpV(const KamataEngine::Vector3& a, const KamataEngine::Vector3& b, float t) {
    return {LerpF(a.x, b.x, t), LerpF(a.y, b.y, t), LerpF(a.z, b.z, t)};
}

void SelectScene::Update() {
   
  
    if (inputTimer_ > 0.0f) inputTimer_ -= 1.0f / 60.0f;

    bool debugSkip = Input::GetInstance()->PushKey(DIK_SPACE) && (Input::GetInstance()->PushKey(DIK_LSHIFT) || Input::GetInstance()->PushKey(DIK_RSHIFT));
    bool padA = KeyInput::GetInstance()->TriggerPadButton(KeyInput::XINPUT_BUTTON_A);
    bool spaceTrig = Input::GetInstance()->TriggerKey(DIK_SPACE);

    // camera update
    if (transitioning_) {
        // drive camera along transition
        transitionTimer_ -= 1.0f / 60.0f;
        float clamped = (transitionTimer_ < 0.0f) ? 0.0f : transitionTimer_;
        transitionProgress_ = 1.0f - (clamped / transitionDuration_);
        // ease in-out (smoothstep)
        float t = transitionProgress_;
        t = t * t * (3.0f - 2.0f * t);
        camera_.translation_ = LerpV(transitionCameraStart_, transitionCameraTarget_, t);
        camera_.UpdateMatrix();
        camera_.TransferMatrix();

        
        if (fade_) fade_->Update();

        if (transitionTimer_ <= 0.0f) {
          
            if (fade_ && !fadeStarted_) {
                fadeStarted_ = true;
                
                fade_->Start(Fade::Status::FadeOut, transitionDuration_ * 0.5f);
            }

          
            if (!fade_ || (fadeStarted_ && fade_->IsFinished())) {
                finished_ = true;
                return;
            }
        }
    } else {
        if (cameraController_) {
            cameraController_->Update();
        } else {
            camera_.UpdateMatrix();
        }
        if (fade_) {
            // keep fade idle (no status) updated to be safe
            fade_->Update();
        }
    }

    
    if (player_ && !transitioning_) {
        player_->Update();
    }

   
    highlightedStage_ = -1;
    if (!stagePositions_.empty() && player_) {
        Vector3 ppos = player_->GetPosition();
        float bestDist2 = std::numeric_limits<float>::infinity();
        int bestIdx = -1;
        for (size_t i = 0; i < stagePositions_.size(); ++i) {
            const Vector3& s = stagePositions_[i];
            float dx = ppos.x - s.x;
            float dy = ppos.y - s.y;
            float d2 = dx*dx + dy*dy;
            if (d2 < bestDist2) {
                bestDist2 = d2;
                bestIdx = static_cast<int>(i);
            }
        }
        if (bestIdx >= 0 && bestDist2 <= stageDetectRadius_ * stageDetectRadius_) {
            highlightedStage_ = bestIdx;
        }
    }

   
    if (debugSkip) {
     
        finished_ = true;
        return;
    }

    if (!transitioning_ && (padA || spaceTrig) && inputTimer_ <= 0.0f) {
        if (highlightedStage_ >= 0) {
           
            chosenStage_ = highlightedStage_;
            transitioning_ = true;
            transitionTimer_ = transitionDuration_;
            transitionProgress_ = 0.0f;
            transitionCameraStart_ = camera_.translation_;
           
            Vector3 s = stagePositions_[chosenStage_];
            // Prevent camera from moving to the stage position to avoid showing empty rows
            // Keep the camera at its current translation for the duration of the transition
            transitionCameraTarget_ = transitionCameraStart_;
          
            inputTimer_ = inputCooldown_;
            return;
        } else {
           
            inputTimer_ = inputCooldown_;
        }
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

   
    for (size_t i = 0; i < stageWorldTransforms_.size(); ++i) {
        WorldTransform* swt = stageWorldTransforms_[i];
        if (!swt) continue;
        if (transitioning_) {
            
            float t = transitionProgress_;
            
            if (static_cast<int>(i) == chosenStage_) {
                float s = LerpF(1.0f, 3.0f, t);
                swt->scale_ = {s, s, s};
            } else {
               
                swt->scale_ = {1.0f, 1.0f, 1.0f};
            }
        } else {
            if (static_cast<int>(i) == highlightedStage_) {
                swt->scale_ = {1.6f, 1.6f, 1.6f};
            } else {
                swt->scale_ = {1.0f, 1.0f, 1.0f};
            }
        }
        swt->matWorld_ = MakeAffineMatrix(swt->scale_, swt->rotation_, swt->translation_);
        if (swt->parent_) swt->matWorld_ = Multiply(swt->parent_->matWorld_, swt->matWorld_);
        swt->TransferMatrix();
        if (stageModel_) {
            stageModel_->Draw(*swt, camera_);
        }
    }

    if (player_ && !transitioning_) player_->Draw();

    Model::PostDraw();

    
    if (fade_) fade_->Draw();
}
