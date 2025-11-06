#include "GameScene.h"
#include "MathUtl.h"
using namespace KamataEngine;

GameScene::GameScene() {}

GameScene::~GameScene() {
#ifdef _DEBUG
	delete debugCamera_;
#endif //  _DEBUG

	delete blockModel_;
	delete cameraController_;
	delete model_;
	delete player_;

	for (std::vector<WorldTransform*>& row : worldTransformBlocks_) {
		for (WorldTransform* wt : row) {
			delete wt;
		}
	}
	worldTransformBlocks_.clear();
}

void GameScene::Initialize() {

#ifdef _DEBUG

	// デバックカメラの生成
	debugCamera_ = new KamataEngine::DebugCamera(kWindowWidth, kWindowHeight);
	
#endif //  _DEBUG

	model_ = Model::Create();
	textureHandle_ = TextureManager::Load("uvChecker.png");

	blockModel_ = Model::Create();
#ifdef _DEBUG
	assert(textureHandle_);
#endif 
	camera_.Initialize();

	player_ = new Player();
	Vector3 playerPosition = {0.0f, 00.0f, 0.0f};
	player_->Initialize(model_, &camera_, playerPosition);

	cameraController_ = new CameraController();
	cameraController_->SetMovableArea({-50.0f, 50.0f, 50.0f, -50.0f});
	cameraController_->Initialize(&camera_);
	cameraController_->SetTarget(player_);
	cameraController_->Reset();

	//ブロックの要素数の変更
	worldTransformBlocks_.resize(kNumBlockVertical);
	for (uint32_t i = 0; i < kNumBlockVertical; ++i)
	{
		worldTransformBlocks_[i].resize(kNumBlockHorizontal);
		// キューブの生成
		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {
			worldTransformBlocks_[i][j] = new WorldTransform();
			worldTransformBlocks_[i][j]->Initialize();
			worldTransformBlocks_[i][j]->translation_.x = kBlockWidth * i;
			worldTransformBlocks_[i][j]->translation_.y = kBlockHeight * j;
		}
	}

	
	
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
	cameraController_->Update();

	for (auto& row : worldTransformBlocks_) {
		for (WorldTransform* wt : row) {
			wt->matWorld_ = MakeAffineMatrix(wt->scale_, wt->rotation_, wt->translation_);
			if (wt->parent_) {
				wt->matWorld_ = Multiply(wt->parent_->matWorld_, wt->matWorld_);
			}
			wt->TransferMatrix();
		}
	}


}

void GameScene::Draw() { 
	
	Model::PreDraw();
	
	player_->Draw(); 

	for (auto& row : worldTransformBlocks_) {
		for (WorldTransform* wt : row) {
			blockModel_->Draw(*wt, camera_);
		}
	}


	Model::PostDraw();
}
