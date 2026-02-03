#include "SelectScene.h"
#include "KeyInput.h"
#include <2d/Sprite.h>
#include <base/TextureManager.h>

#include "Player.h"
#include "MapChipField.h"
#include "CameraController.h"
#include "Fade.h"
#include "Skydome.h"

#include <algorithm>
#include <cmath>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

using namespace KamataEngine;

SelectScene::~SelectScene() {
    // stop BGM via Audio
    if (bgmStarted_) {
        Audio::GetInstance()->StopWave(bgmVoiceHandle_);
        bgmVoiceHandle_ = 0u;
        bgmStarted_ = false;
    }

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

    // delete the special Stage1 model if created
    if (stage1Model_) {
        delete stage1Model_;
        stage1Model_ = nullptr;
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

    // release stage number sprites
    for (Sprite* sp : stageNumberSprites_) {
        delete sp;
    }
    stageNumberSprites_.clear();
    stageNumberTexHandles_.clear();

    //// release left-bottom UI sprite
    //if (ltSprite_) {
    //    delete ltSprite_;
    //    ltSprite_ = nullptr;
    //}
    //ltTexHandle_ = 0u;

    //// release Q sprite
    //if (qSprite_) {
    //    delete qSprite_;
    //    qSprite_ = nullptr;
    //}
    //qTexHandle_ = 0u;

    // release skydome
    if (skydome_) {
        delete skydome_;
        skydome_ = nullptr;
    }

    if (fade_) {
        delete fade_;
        fade_ = nullptr;
    }
}

void SelectScene::Initialize() {
    
    camera_.Initialize();
    // extend camera far plane so distant skydome and stage are visible
    camera_.farZ = 3000.0f;
    camera_.translation_ = {0.0f, 0.0f, -50.0f};
    camera_.UpdateMatrix();
    camera_.TransferMatrix();

    
    cameraController_ = new CameraController();

    // Skydome
    skydome_ = new Skydome();
    skydome_->Initialize();
    skydome_->SetCamera(&camera_);
    skydome_->SetRotationSpeed(0.0f); // if needed, set slow rotation

    
    mapChipField_ = new MapChipField();
    
    mapChipField_->LoadMapChipCsv("Resources/Map/SelectScene/SelectScene.csv");

    
    blockModel_ = Model::CreateFromOBJ("Block");
    
    // default stage model
    stageModel_ = Model::CreateFromOBJ("Stage");
    // load special Stage1/2/3 models (if exist) so we can use specific visuals per stage
    // Attempt multiple candidate names so models placed in different resource layouts are found
    stage1Model_ = nullptr;
    const char* stage1Candidates[] = {
        // simple names
        "Stage1", "stage1",
        // common folder/name combos
        "Stage1/Stage", "Stage1/stage", "stage1/Stage", "stage1/stage",
        "Stage/Stage1", "stage/stage1",
        // same-name folder
        "Stage1/Stage1", "stage1/stage1",
        // other variations
        "Stage_1", "stage_1", "StageOne", "stageone"
    };
    for (const char* cand : stage1Candidates) {
        stage1Model_ = Model::CreateFromOBJ(cand);
        if (stage1Model_) {
            DebugText::GetInstance()->ConsolePrintf("SelectScene: loaded Stage1 model using '%s'\n", cand);
            break;
        }
    }
    if (!stage1Model_) {
        DebugText::GetInstance()->ConsolePrintf("SelectScene: Stage1 model not found, will use default Stage.obj\n");
    }

    // Stage2
    stage2Model_ = nullptr;
    const char* stage2Candidates[] = {
        "Stage2", "stage2",
        "Stage2/Stage", "Stage2/stage", "stage2/Stage", "stage2/stage",
        "Stage/Stage2", "stage/stage2",
        "Stage2/Stage2", "stage2/stage2",
        "Stage_2", "stage_2", "StageTwo", "stagetwo"
    };
    for (const char* cand : stage2Candidates) {
        stage2Model_ = Model::CreateFromOBJ(cand);
        if (stage2Model_) {
            DebugText::GetInstance()->ConsolePrintf("SelectScene: loaded Stage2 model using '%s'\n", cand);
            break;
        }
    }
    if (!stage2Model_) {
        DebugText::GetInstance()->ConsolePrintf("SelectScene: Stage2 model not found, will use default Stage.obj\n");
    }

    // Stage3
    stage3Model_ = nullptr;
    const char* stage3Candidates[] = {
        "Stage3", "stage3",
        "Stage3/Stage", "Stage3/stage", "stage3/Stage", "stage3/stage",
        "Stage/Stage3", "stage/stage3",
        "Stage3/Stage3", "stage3/stage3",
        "Stage_3", "stage_3", "StageThree", "stagethree"
    };
    for (const char* cand : stage3Candidates) {
        stage3Model_ = Model::CreateFromOBJ(cand);
        if (stage3Model_) {
            DebugText::GetInstance()->ConsolePrintf("SelectScene: loaded Stage3 model using '%s'\n", cand);
            break;
        }
    }
    if (!stage3Model_) {
        DebugText::GetInstance()->ConsolePrintf("SelectScene: Stage3 model not found, will use default Stage.obj\n");
    }

    textureHandleStage1_ = TextureManager::Load("Number/1.png");
    textureHandleStage2_ = TextureManager::Load("Number/2.png");
    textureHandleStage3_ = TextureManager::Load("Number/3.png");

    
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
                    // push stage node slightly behind the player (player z=0, negative is closer)
                    // so use small positive z to place behind
                    swt->translation_.z += 1.0f;
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

    // Create HUD sprites for stage numbers (left->right = 1,2,3,...)
    if (!stageWorldTransforms_.empty()) {
        size_t count = stageWorldTransforms_.size();
        stageNumberTexHandles_.reserve(count);
        stageNumberSprites_.reserve(count);
        for (size_t i = 0; i < count; ++i) {
            // Try multiple common file naming schemes; first successful load will be used
            uint32_t h = 0;
            // 1-based index
            char pathA[128];
            sprintf_s(pathA, "Number/%zu.png", i + 1); // Resources/Number/{n}.png
            h = TextureManager::Load(pathA);
            if (h == 0) {
                char path1[128];
                sprintf_s(path1, "Textures/Select/%zu.png", i + 1);
                h = TextureManager::Load(path1);
            }
            if (h == 0) {
                char path2[128];
                sprintf_s(path2, "UI/Select/%zu.png", i + 1);
                h = TextureManager::Load(path2);
            }
            stageNumberTexHandles_.push_back(h);
            Sprite* sp = Sprite::Create(h, {0.0f, 0.0f});
            // size and anchor
            sp->SetSize({96.0f, 96.0f});
            sp->SetAnchorPoint({0.5f, 0.5f});
            stageNumberSprites_.push_back(sp);
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
    // start fade-in so scene appears by gradually becoming visible
    fade_->Start(Fade::Status::FadeIn, 0.6f);

    // Load select scene BGM (relative to Resources/)
    bgmDataHandle_ = Audio::GetInstance()->LoadWave("Audio/BGM/SelectScene.wav");
}

static float LerpF(float a, float b, float t) { return a + (b - a) * t; }
static KamataEngine::Vector3 LerpV(const KamataEngine::Vector3& a, const KamataEngine::Vector3& b, float t) {
    return {LerpF(a.x, b.x, t), LerpF(a.y, b.y, t), LerpF(a.z, b.z, t)};
}

void SelectScene::Update() {
    
    
    if (inputTimer_ > 0.0f) inputTimer_ -= 1.0f / 60.0f;

    // Ensure BGM has started once
    if (!bgmStarted_) {
        bgmVoiceHandle_ = Audio::GetInstance()->PlayWave(bgmDataHandle_, true, 0.8f);
        bgmStarted_ = true;
    }

    bool debugSkip = Input::GetInstance()->PushKey(DIK_SPACE) && (Input::GetInstance()->PushKey(DIK_LSHIFT) || Input::GetInstance()->PushKey(DIK_RSHIFT));
    bool padA = KeyInput::GetInstance()->TriggerPadButton(KeyInput::XINPUT_BUTTON_A);
    bool spaceTrig = Input::GetInstance()->TriggerKey(DIK_SPACE);

    // Detect input device use this frame and persist it to lastInputMode_
    // Gamepad: any pad button or left stick moved
    bool padUsed = KeyInput::GetInstance()->PushPadButton(KeyInput::XINPUT_BUTTON_A) ||
                   std::abs(KeyInput::GetInstance()->GetLStick().x) > 0.001f ||
                   std::abs(KeyInput::GetInstance()->GetLStick().y) > 0.001f;
    // Keyboard: any key pressed this frame (check whole key buffer)
    bool anyKeyPressed = false;
    const auto& allKeys = Input::GetInstance()->GetAllKey();
    for (BYTE v : allKeys) {
        if (v) { anyKeyPressed = true; break; }
    }
    if (padUsed) {
        lastInputMode_ = SelectScene::InputMode::kGamepad;
    } else if (anyKeyPressed) {
        lastInputMode_ = SelectScene::InputMode::kKeyboard;
    }

    // Update fade once at frame start if still fading in (do not block camera/player updates)
    bool fadeUpdatedThisFrame = false;
    if (fade_ && !transitioning_ && !fade_->IsFinished()) {
        fade_->Update();
        fadeUpdatedThisFrame = true;
    }

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

        
        if (fade_ && !fadeUpdatedThisFrame) fade_->Update();

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
            // ensure camera matrices are transferred after controller updates
            camera_.TransferMatrix();
        } else {
            camera_.UpdateMatrix();
            camera_.TransferMatrix();
        }
        if (fade_ && !fadeUpdatedThisFrame) {
            // keep fade idle (no status) updated to be safe
            fade_->Update();
        }
    }

    
    if (player_ && !transitioning_) {
        // If the confirm input was pressed this frame (space or gamepad A), suppress the player's next jump
        if (padA || spaceTrig) {
            player_->SuppressNextJump();
        }
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

    
    if (debugSkip && (fade_ == nullptr || fade_->IsFinished())) {
        
        finished_ = true;
        return;
    }

    if (!transitioning_ && (fade_ == nullptr || fade_->IsFinished()) && (padA || spaceTrig) && inputTimer_ <= 0.0f) {
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

    // Skydome first
    if (skydome_) {
        skydome_->Update();
        skydome_->Draw();
    }

    
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
                // Lift up and add a gentle tilt instead of large scaling
                float lift = LerpF(0.0f, 2.0f, t);
                swt->translation_.y += lift;
                // small tilt around Z for a bit of flair
                swt->rotation_.z = LerpF(0.0f, 0.35f, t);
                // only a subtle scale up
                float s = LerpF(1.0f, 1.2f, t);
                swt->scale_ = {s, s, s};
            } else {
                // keep others at normal scale
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
            // draw stage model (use specific models for 1st/2nd/3rd if available)
            Model* modelToDraw = stageModel_;
            if (static_cast<int>(i) == 0 && stage1Model_) {
                modelToDraw = stage1Model_;
            } else if (static_cast<int>(i) == 1 && stage2Model_) {
                modelToDraw = stage2Model_;
            } else if (static_cast<int>(i) == 2 && stage3Model_) {
                modelToDraw = stage3Model_;
            }
            modelToDraw->Draw(*swt, camera_);
        }
    }

    if (player_ && !transitioning_) player_->Draw();

    Model::PostDraw();

    // Draw stage number PNGs on HUD, aligned left-to-right (1,2,3,...)
    //if (!stageNumberSprites_.empty()) {
    //    Sprite::PreDraw(nullptr, Sprite::BlendMode::kNormal);
    //    const float startX = 64.0f;
    //    const float gapX = 120.0f;
    //    const float y = 64.0f;
    //    for (size_t i = 0; i < stageNumberSprites_.size(); ++i) {
    //        Sprite* sp = stageNumberSprites_[i];
    //        if (!sp) continue;
    //        float x = startX + gapX * static_cast<float>(i);
    //        sp->SetPosition({x, y});
    //        // slightly enlarge highlighted
    //        if (static_cast<int>(i) == highlightedStage_ && !transitioning_) {
    //            sp->SetSize({112.0f, 112.0f});
    //        } else {
    //            sp->SetSize({96.0f, 96.0f});
    //        }
    //        sp->Draw();
    //    }
    //    Sprite::PostDraw();
    //}

    //{
    //    Sprite* toDraw = nullptr;
    //    if (lastInputMode_ == SelectScene::InputMode::kGamepad && ltSprite_) {
    //        toDraw = ltSprite_;
    //    } else if (lastInputMode_ == SelectScene::InputMode::kKeyboard && qSprite_) {
    //        toDraw = qSprite_;
    //    }

    //    // fallback: if unknown, prefer keyboard sprite if exists
    //   

    //    if (toDraw) {
    //        int h = DirectXCommon::GetInstance()->GetBackBufferHeight();
    //        Sprite::PreDraw(nullptr, Sprite::BlendMode::kNormal);
    //        // place at 10 px margin from left and bottom
    //        toDraw->SetPosition({10.0f, static_cast<float>(h) - 10.0f});
    //        toDraw->Draw();
    //        Sprite::PostDraw();
    //    }
    //}

    // draw fade on top
    if (fade_) fade_->Draw();
}

void SelectScene::SuppressPlayerNextJump() {
    if (player_) player_->SuppressNextJump();
}
