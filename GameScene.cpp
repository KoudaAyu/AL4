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
	Vector3 playerPosition = {0.0f, 00.0f, 0.0f};
	player_->Initialize(model_,&camera_,playerPosition);
}

void GameScene::Update() {

#ifdef _DEBUG

	ImGui::Begin("Window");
	const Vector3& pos = player_->GetPosition();
	ImGui::Text("Player Position: X=%.2f, Y=%.2f, Z=%.2f", pos.x, pos.y, pos.z);

	ImGui::End();

	debugCamera_->Update();

	AxisIndicator::GetInstance()->SetVisible(true);
	AxisIndicator::GetInstance()->SetTargetCamera(&debugCamera_->GetCamera());

#endif //  _DEBUG

	player_->Update();

}

void GameScene::Draw() { 
	
	Model::PreDraw();
	
	player_->Draw(); 

	Model::PostDraw();
}
