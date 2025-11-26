#include "GameScene.h"
#include "AABB.h"
#include "MapChipField.h"
#include "MathUtl.h"
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
	delete enemy_;
	delete player_;

	delete deathParticle_;

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
	player_->Initialize(&camera_, playerPosition);
	player_->SetMapChipField(mapChipField_);

	Vector3 enemyPosition = {12.0f, 2.0f, 0.0f};
	if (mapChipField_) {
		bool found = false;
		uint32_t vh = mapChipField_->GetNumBlockVertical();
		uint32_t wh = mapChipField_->GetNumBlockHorizontal();
		for (uint32_t y = 0; y < vh && !found; ++y) {
			for (uint32_t x = 0; x < wh; ++x) {
				if (mapChipField_->GetMapChipTypeByIndex(x, y) == MapChipType::kEnemySpawn) {
					enemyPosition = mapChipField_->GetMapChipPositionByIndex(x, y);
					found = true;
					break;
				}
			}
		}
	}

	enemy_ = new Enemy();
	enemy_->Initialize(model_, &camera_, enemyPosition);

	cameraController_ = new CameraController();
	// Set movable area based on loaded map CSV instead of hardcoded values
	if (mapChipField_) {
		cameraController_->SetMovableArea(mapChipField_->GetMovableArea());
	} else {
		cameraController_->SetMovableArea({-50.0f, 50.0f, 50.0f, -50.0f});
	}
	cameraController_->Initialize(&camera_);
	cameraController_->SetTarget(player_);
	// プレイヤーにカメラコントローラ参照を渡す（初回割当て）
	if (player_) {
		player_->SetCameraController(cameraController_);
	}
	cameraController_->Reset();

	// CSV に従ってブロック生成（全マス生成は行わない）
	GenerateBlocks();

	// Skydome の生成と初期化
	skydome_ = new Skydome();
	skydome_->Initialize();
	skydome_->SetCamera(&camera_);

	// Particle関係
	deathParticle_ = new DeathParticle();
	deathParticle_->Initialize(model_, &camera_, playerPosition);
}

void GameScene::Update() {

	// リセットキーでシーンをリセット
	if (Input::GetInstance()->TriggerKey(DIK_R)) {
		Reset();
		// Reset() may delete deathParticle_ and other objects; stop further Update this frame to avoid using freed memory
		return;
	}




	switch (phase_) {
	case Phase::kPlay:

		#ifdef _DEBUG

		ImGui::Begin("Window");

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
		if (isDebugCameraActive_) {
			debugCamera_->Update();
			camera_.matView = debugCamera_->GetCamera().matView;
			camera_.matProjection = debugCamera_->GetCamera().matProjection;
			camera_.TransferMatrix();
		} else {
			// 通常時はカメラコントローラがカメラを更新
			cameraController_->Update();
			camera_.UpdateMatrix();
		}

#endif //  _DEBUG

		skydome_->Update();

		enemy_->Update();

		player_->Update();

		for (auto& row : worldTransformBlocks_) {
			for (WorldTransform* wt : row) {
				if (!wt) {
					continue;
				}
				wt->matWorld_ = MakeAffineMatrix(wt->scale_, wt->rotation_, wt->translation_);
				if (wt->parent_) {
					wt->matWorld_ = Multiply(wt->parent_->matWorld_, wt->matWorld_);
				}
				wt->TransferMatrix();
			}
		}

#ifndef _DEBUG
		cameraController_->Update();
		camera_.UpdateMatrix();
#endif //  _DEBUG

		CheckAllCollisions();

		/*if (!enemy_->isAlive())
		{
			delete enemy_;
		}
		*/

		// フェーズ切り替えをチェック
		ChangePhase();
		break;
	case Phase::kDeath:

		#ifdef _DEBUG

		ImGui::Begin("Window");

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
		if (isDebugCameraActive_) {
			debugCamera_->Update();
			camera_.matView = debugCamera_->GetCamera().matView;
			camera_.matProjection = debugCamera_->GetCamera().matProjection;
			camera_.TransferMatrix();
		} else {
			// 通常時はカメラコントローラがカメラを更新
			cameraController_->Update();
			camera_.UpdateMatrix();
		}

#endif //  _DEBUG

		skydome_->Update();
		enemy_->Update();

		// Particle関係
		if (deathParticle_) {
			deathParticle_->Update();
		}

#ifndef _DEBUG
		cameraController_->Update();
		camera_.UpdateMatrix();
#endif //  _DEBUG

		for (auto& row : worldTransformBlocks_) {
			for (WorldTransform* wt : row) {
				if (!wt) {
					continue;
				}
				wt->matWorld_ = MakeAffineMatrix(wt->scale_, wt->rotation_, wt->translation_);
				if (wt->parent_) {
					wt->matWorld_ = Multiply(wt->parent_->matWorld_, wt->matWorld_);
				}
				wt->TransferMatrix();
			}
		}

		break;
	}
}

void GameScene::Draw() {

	Model::PreDraw();

	// 先にスカイドームを描画
	if (skydome_) {
		skydome_->Draw();
	}

	enemy_->Draw();

	// デス中はプレイヤーの描画を抑制してエフェクトを見やすくする
	if (phase_ != Phase::kDeath) {
		player_->Draw();
	}

	for (auto& row : worldTransformBlocks_) {
		for (WorldTransform* wt : row) {
			if (!wt) {
				continue;
			}
			blockModel_->Draw(*wt, camera_);
		}
	}

	// Particle関係
	if (deathParticle_) {
		deathParticle_->Draw();
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

	for (uint32_t i = 0; i < numBlockVirtical; ++i) {
		for (uint32_t j = 0; j < numBlockHorizontal; ++j) {
			if (mapChipField_->GetMapChipTypeByIndex(j, i) == MapChipType::kBlock) {
				WorldTransform* worldTransform = new WorldTransform();
				worldTransform->Initialize();
				worldTransform->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
				worldTransformBlocks_[i][j] = worldTransform;
			}
		}
	}
}

void GameScene::CheckAllCollisions() {
#pragma region プレイヤーと敵の当たり判定

	// 敵またはプレイヤーが死亡している場合は衝突判定をスキップ
	//敵を複数追加したら消す
	if (!player_ || !enemy_ || !player_->isAlive() || !enemy_->isAlive()) {
		return;
	}

	if (IsCollisionAABBAABB(player_->GetAABB(), enemy_->GetAABB())) {

		player_->OnCollision(enemy_);
		enemy_->OnCollision(player_);
	
	}

#pragma endregion
}

void GameScene::ChangePhase() {

	switch (phase_) {
	case Phase::kPlay:

		if (!player_->isAlive())
		{
			phase_ = Phase::kDeath;
			const Vector3& deathPos = player_->GetPosition();

			if (deathParticle_)
			{
				delete deathParticle_;
				deathParticle_ = nullptr;
			}

			deathParticle_ = new DeathParticle();
			deathParticle_->Initialize(model_, &camera_, deathPos);
		}

		break;
	case Phase::kDeath:

		break;
	}
}

// リセット処理
void GameScene::Reset() {
	// Delete existing player and recreate
	if (player_) {
		delete player_;
		player_ = nullptr;
	}
	Vector3 playerPosition = {4.0f, 4.0f, 0.0f};
	player_ = new Player();
	player_->Initialize(&camera_, playerPosition);
	player_->SetMapChipField(mapChipField_);
	// 再生成したプレイヤーにもカメラコントローラを渡す
	if (cameraController_) {
		player_->SetCameraController(cameraController_);
	}

	// Delete existing enemy and recreate
	if (enemy_) {
		delete enemy_;
		enemy_ = nullptr;
	}
	Vector3 enemyPosition = {12.0f, 2.0f, 0.0f};
	if (mapChipField_) {
		bool found = false;
		uint32_t vh = mapChipField_->GetNumBlockVertical();
		uint32_t wh = mapChipField_->GetNumBlockHorizontal();
		for (uint32_t y = 0; y < vh && !found; ++y) {
			for (uint32_t x = 0; x < wh; ++x) {
				if (mapChipField_->GetMapChipTypeByIndex(x, y) == MapChipType::kEnemySpawn) {
					enemyPosition = mapChipField_->GetMapChipPositionByIndex(x, y);
					found = true;
					break;
				}
			}
		}
	}
	enemy_ = new Enemy();
	enemy_->Initialize(model_, &camera_, enemyPosition);

	if (deathParticle_) {
		delete deathParticle_;
		deathParticle_ = nullptr;
	}

	GenerateBlocks();

	if (cameraController_) {
		cameraController_->SetTarget(player_);
		cameraController_->Reset();
	}

	phase_ = Phase::kPlay;
}
