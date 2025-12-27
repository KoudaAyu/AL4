#include "SelectScene.h"
#include "KeyInput.h"
#include <2d/Sprite.h>

using namespace KamataEngine;

SelectScene::~SelectScene() {}

void SelectScene::Initialize() {
    // placeholder initialization
}

void SelectScene::Update() {
    // Proceed to game when SPACE or gamepad A is pressed
    if (Input::GetInstance()->PushKey(DIK_SPACE) || KeyInput::GetInstance()->TriggerPadButton(KeyInput::XINPUT_BUTTON_A)) {
        finished_ = true;
    }
}

void SelectScene::Draw() {
    // nothing for now
}
