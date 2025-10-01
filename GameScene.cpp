#include "GameScene.h"

using namespace KamataEngine;

GameScene::GameScene() {}

GameScene::~GameScene() {
#ifdef _DEBUG
	delete debugCamera_;
#endif //  _DEBUG

	delete model_;
	delete player_;
}

void GameScene::Initialize() {

#ifdef _DEBUG

	// デバックカメラの生成
	debugCamera_ = new KamataEngine::DebugCamera(kWindowWidth, kWindowHeight);

#endif //  _DEBUG

	model_ = Model::Create();
	textureHandle_ = TextureManager::Load("uvChecker.png");
	assert(textureHandle_);
	camera_.Initialize();
	

	player_ = new Player();
	player_->Initialize(model_,textureHandle_,&camera_);
}

void GameScene::Update() {

#ifdef _DEBUG

	ImGui::Begin("Window");
	ImGui::End();

	debugCamera_->Update();

	AxisIndicator::GetInstance()->SetVisible(true);
	AxisIndicator::GetInstance()->SetTargetCamera(&debugCamera_->GetCamera());

#endif //  _DEBUG

	player_->Uppdate();

}

void GameScene::Draw() { 
	
	Model::PreDraw();
	
	player_->Draw(); 

	Model::PostDraw();
}
