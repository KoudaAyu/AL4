#include "TitleScene.h"
#include "MathUtl.h"
#include "KeyInput.h"
#include <cassert>
#include <numbers>
#include <algorithm>
#include "Skydome.h"

using namespace KamataEngine;

TitleScene::~TitleScene() {
    delete fade_;
    if (model_) {
        delete model_;
        model_ = nullptr;
    }
    if (particle_) {
        delete particle_;
        particle_ = nullptr;
    }
    if (particleModel_) {
        delete particleModel_;
        particleModel_ = nullptr;
    }
    if (skydome_) {
        delete skydome_;
        skydome_ = nullptr;
    }
}

void TitleScene::Initialize() {

    fade_ = new Fade();
    fade_->Initialize();
    fade_->Start(Fade::Status::FadeIn, 3.0f);

    // Create title model from OBJ placed in Resources/Models/Title.obj (name: "title")
    model_ = Model::CreateFromOBJ("title", true);
    // Ensure model was created in debug builds
    #ifdef _DEBUG
    assert(model_);
    #endif

    // Create particle model from OBJ (Nikukyuu)
    particleModel_ = Model::CreateFromOBJ("Nikukyuu", true);
    #ifdef _DEBUG
    assert(particleModel_);
    #endif

    // Initialize camera for title scene
    camera_.Initialize();
    // match GameScene farZ
    camera_.farZ = 3000.0f;
    camera_.UpdateProjectionMatrix();
    // Place camera so title model is visible: use default farther distance
    camera_.translation_ = {0.0f, 0.0f, cameraStartZ_};
    camera_.UpdateMatrix();
    camera_.TransferMatrix();

    // World transform for title model
    worldTransform_.Initialize();
    worldTransform_.translation_ = {0.0f, 0.0f, 0.0f};
    // Rotate model so it faces the camera (fix model that is pointing upward)
    worldTransform_.rotation_.x = std::numbers::pi_v<float> / 2.0f; // flipped from -pi/2 to +pi/2 to correct upside-down text
    // Rotate 180 degrees around Y to flip front/back so text faces the camera
    worldTransform_.rotation_.y = std::numbers::pi_v<float>;
    // Increase scale so model is likely visible
    worldTransform_.scale_ = {5.0f, 5.0f, 5.0f};

    // initialize effect state
    effectTimer_ = 0.0f;
    particle_ = nullptr;

    // rotation speed defaults
    rotationSpeed_ = 0.5f;
    targetRotationSpeed_ = 0.5f;
    rotationLerpSpeed_ = 8.0f;

    // create and initialize skydome
    skydome_ = new Skydome();
    skydome_->Initialize();
    skydome_->SetCamera(&camera_);

}

void TitleScene::Update() {

    const float dt = 1.0f / 60.0f;

    switch (phase_)
    {
    case Phase::kFadeIn:
        // advance fade once per frame
        fade_->Update();

        if (fade_->IsFinished()) {
            phase_ = Phase::kMain;
        }

        break;

        case Phase::kMain:
        // accept space key or Xbox A button
        if (Input::GetInstance()->PushKey(DIK_SPACE) || KeyInput::GetInstance()->TriggerPadButton(KeyInput::XINPUT_BUTTON_A)) {
            // start the short effect animation before fading out: spawn particles and boost rotation
            effectTimer_ = 0.0f;
            if (!particle_) {
                particle_ = new DeathParticle();
                // use particle-specific model
                particle_->Initialize(particleModel_, &camera_, worldTransform_.translation_);
            }
            // boost rotation speed target
            targetRotationSpeed_ = 0.5f + rotationBoost_; // temporary high speed
            phase_ = Phase::kEffect;
        }

        break;

        case Phase::kEffect:
        // play a short particle + rotation effect
        effectTimer_ += dt;
        if (particle_) particle_->Update();

        {
            float t = effectTimer_ / effectDuration_;
            if (t > 1.0f) t = 1.0f;

            // during effect we keep the boosted targetRotationSpeed_ (set on key press)
            // when effect completes, restore the target to baseline
            if (t >= 1.0f) {
                // start fade out after effect completes
                phase_ = Phase::kFadeOut;
                fade_->Start(Fade::Status::FadeOut, 3.0f);
                // restore rotation speed target back to baseline
                targetRotationSpeed_ = 0.5f;
            }
        }

        break;

        case Phase::kFadeOut:
            // advance fade once per frame so it can finish
            // continue updating particle effect during fade out
            if (particle_) particle_->Update();
            fade_->Update();
            if (fade_->IsFinished()) {
                finished_ = true;
            }

            break;
    }

    // smoothly interpolate rotation speed toward target
    {
        float lerpT = std::clamp(rotationLerpSpeed_ * dt, 0.0f, 1.0f);
        rotationSpeed_ += (targetRotationSpeed_ - rotationSpeed_) * lerpT;
        // apply rotation using current rotationSpeed_
        worldTransform_.rotation_.y += rotationSpeed_ * dt;
    }

    worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
    worldTransform_.TransferMatrix();

    // update skydome so it follows camera if needed
    if (skydome_) {
        skydome_->Update();
    }

}

void TitleScene::Draw() {

    Model::PreDraw();
    // draw skydome first
    if (skydome_) skydome_->Draw();

    if (model_) {
        model_->Draw(worldTransform_, camera_);
    }
    // draw particles if spawned
    if (particle_) {
        particle_->Draw();
    }
    Model::PostDraw();

    fade_->Draw();

}
