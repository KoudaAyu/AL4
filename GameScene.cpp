#include "GameScene.h"
#include "MathUtl.h"
#include"MapChipField.h"
using namespace KamataEngine;

GameScene::GameScene() {}

GameScene::~GameScene() {
#ifdef _DEBUG
	delete debugCamera_;
#endif //  _DEBUG

	delete blockModel_;
	delete cameraController_;
	delete mapChipField_;
	delete model_;
	delete player_;

	for (std::vector<WorldTransform*>& row : worldTransformBlocks_) {
		for (WorldTransform* wt : row) {
			delete wt;
		}
	}
	worldTransformBlocks_.clear();

	delete skydome_;
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

	mapChipField_ = new MapChipField();
	mapChipField_->LoadMapChipCsv("Resources/Debug/Map/Block.csv");

	player_ = new Player();
	Vector3 playerPosition = {4.0f, 4.0f, 0.0f};
	player_->Initialize(model_, &camera_, playerPosition);
	player_->SetMapChipField(mapChipField_);

	cameraController_ = new CameraController();
	cameraController_->SetMovableArea({-50.0f, 50.0f, 50.0f, -50.0f});
	cameraController_->Initialize(&camera_);
	cameraController_->SetTarget(player_);
	cameraController_->Reset();

	

	// CSV に従ってブロック生成（全マス生成は行わない）
	GenerateBlocks();

	// Skydome の生成と初期化
	skydome_ = new Skydome();
	skydome_->Initialize();
	skydome_->SetCamera(&camera_);
}

void GameScene::Update() {

#ifdef _DEBUG

	ImGui::Begin("Window");
	const Vector3& pos = player_->GetPosition();
	ImGui::Text("Player Position: X=%.2f, Y=%.2f, Z=%.2f", pos.x, pos.y, pos.z);

	ImGui::End();

	// トグル
	if (Input::GetInstance()->TriggerKey(DIK_C)) {
		isDebugCameraActive_ = !isDebugCameraActive_;
	}

	// 軸インジケータの表示と対象カメラ設定
	AxisIndicator::GetInstance()->SetVisible(true);
	if (isDebugCameraActive_) {
		AxisIndicator::GetInstance()->SetTargetCamera(&debugCamera_->GetCamera());
	} else {
		AxisIndicator::GetInstance()->SetTargetCamera(&camera_);
	}

	// デバッグカメラ有効時はデバッグカメラの行列をゲーム用カメラへコピー
	if (isDebugCameraActive_)
	{
		debugCamera_->Update();
		camera_.matView = debugCamera_->GetCamera().matView;
		camera_.matProjection = debugCamera_->GetCamera().matProjection;
		camera_.TransferMatrix();
	}
	else
	{
		// 通常時はカメラコントローラがカメラを更新
		cameraController_->Update();
		camera_.UpdateMatrix();
	}

#else
	// リリースビルドでは常に通常カメラを更新
	cameraController_->Update();
	camera_.UpdateMatrix();
#endif //  _DEBUG

	player_->Update();

	for (auto& row : worldTransformBlocks_) {
		for (WorldTransform* wt : row) {
			if (!wt) { continue; }
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

	// 先にスカイドームを描画
	if (skydome_) { skydome_->Draw(); }
	
	player_->Draw(); 

	for (auto& row : worldTransformBlocks_) {
		for (WorldTransform* wt : row) {
			if (!wt) { continue; }
			blockModel_->Draw(*wt, camera_);
		}
	}


	Model::PostDraw();
}

void GameScene::GenerateBlocks() {

	uint32_t numBlockVirtical = mapChipField_->GetNumBlockVertical();
	uint32_t numBlockHorizontal = mapChipField_->GetNumBlockHorizontal();

	// 既存を破棄しクリア
	for (auto& row : worldTransformBlocks_) {
		for (WorldTransform* wt : row) {
			delete wt;
		}
	}
	worldTransformBlocks_.clear();

	// 要素数を設定し、nullptr で初期化
	worldTransformBlocks_.assign(numBlockVirtical, std::vector<WorldTransform*>(numBlockHorizontal, nullptr));

	for (uint32_t i = 0; i < numBlockVirtical; ++i)
	{
		for (uint32_t j = 0; j < numBlockHorizontal; ++j)
		{
			if (mapChipField_->GetMapChipTypeByIndex(j,i) == MapChipType::kBlock)
			{
				WorldTransform* worldTransform = new WorldTransform();
				worldTransform->Initialize();
				worldTransform->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
				worldTransformBlocks_[i][j] = worldTransform;
			}
		}
	}
}
