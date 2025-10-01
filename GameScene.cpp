#include "GameScene.h"

using namespace KamataEngine;

GameScene::GameScene() {}

GameScene::~GameScene() {

	delete debugCamera_;

}

void GameScene::Initialize() {

	// デバックカメラの生成
	debugCamera_ = new KamataEngine::DebugCamera(kWindowWidth, kWindowHeight);


}

void GameScene::Update() {

#ifdef _DEBUG

	ImGui::Begin("Window");
	ImGui::End();

	debugCamera_->Update();

	AxisIndicator::GetInstance()->SetVisible(true);
	AxisIndicator::GetInstance()->SetTargetCamera(&debugCamera_->GetCamera());

#endif //  _DEBUG

	

}

void GameScene::Draw() {}
