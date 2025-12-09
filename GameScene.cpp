#include "GameScene.h"
#include "AABB.h"
#include "FrontShieldEnemy.h"
#include "MapChipField.h"
#include "MathUtl.h"
#include <2d/Sprite.h>
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
	// delete all enemies
	for (Enemy* enemy : enemies_) {
		delete enemy;
	}
	enemies_.clear();
	delete player_;

	delete deathParticle_;

	for (std::vector<WorldTransform*>& row : worldTransformBlocks_) {
		for (WorldTransform* wt : row) {
			delete wt;
		}
	}
	worldTransformBlocks_.clear();

	delete skydome_;

	// HUD sprite
	if (hudSprite_) {
		delete hudSprite_;
		hudSprite_ = nullptr;
	}
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

	// Create enemies based on map chip CSV spawn points
	if (mapChipField_) {
		uint32_t vh = mapChipField_->GetNumBlockVertical();
		uint32_t wh = mapChipField_->GetNumBlockHorizontal();
		for (uint32_t y = 0; y < vh; ++y) {
			for (uint32_t x = 0; x < wh; ++x) {
				MapChipType t = mapChipField_->GetMapChipTypeByIndex(x, y);
				if (t == MapChipType::kEnemySpawn) {
					Vector3 enemyPosition = mapChipField_->GetMapChipPositionByIndex(x, y);
					Enemy* enemy = new Enemy();
					// Use overload where Enemy creates/owns its own model
					enemy->Initialize(&camera_, enemyPosition);
					enemies_.push_back(enemy);
				} else if (t == MapChipType::kEnemySpawnShield) {
					// We use CSV value '4' mapped to kEnemySpawnShield to indicate shield enemy spawn
					Vector3 enemyPosition = mapChipField_->GetMapChipPositionByIndex(x, y);
					FrontShieldEnemy* fse = new FrontShieldEnemy();
					// Use overload where FrontShieldEnemy creates/owns its own model
					fse->Initialize(&camera_, enemyPosition);
					// optional: adjust threshold for variety
					fse->SetFrontDotThreshold(0.6f);
					enemies_.push_back(fse);
				}
			}
		}
	}

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

	// HUD: load PNG and create sprite for release build
	// Put your PNG under Resources/UI/hud.png
	hudTextureHandle_ = TextureManager::Load("Debug/Goal.png");
	if (hudTextureHandle_ != 0u) {
		// Create sprite anchored at bottom-center
		const float hudWidth = 250.0f;
		const float hudHeight = 30.0f;
		const float hudMargin = 20.0f;
		// Position at bottom-center using window constants
		hudSprite_ = KamataEngine::Sprite::Create(hudTextureHandle_, KamataEngine::Vector2{static_cast<float>(kWindowWidth) / 2.0f, static_cast<float>(kWindowHeight) - hudMargin}, KamataEngine::Vector4{1,1,1,1}, KamataEngine::Vector2{0.5f, 1.0f});
		if (hudSprite_) {
			hudSprite_->SetSize(KamataEngine::Vector2{hudWidth, hudHeight});
		}
	}
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

		// Update all enemies
		for (Enemy* enemy : enemies_) {
			if (enemy)
				enemy->Update();
		}

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

		// If all enemies are defeated, finish this scene to return to title
		{
			bool anyAlive = false;
			for (Enemy* enemy : enemies_) {
				if (enemy && enemy->isAlive()) {
					anyAlive = true;
					break;
				}
			}
			// Finish the scene when there are no living enemies.
			// (Previously required the player to be alive as well; remove that to allow switching even if player died.)
			if (!anyAlive) {
				finished_ = true;
				return;
			}
		}

		// HUD update: keep HUD positioned relative to screen if sprite exists
		if (hudSprite_) {
			// Keep it anchored bottom-center in case window size changes
			const float hudMargin = 20.0f;
			hudSprite_->SetPosition(KamataEngine::Vector2{static_cast<float>(kWindowWidth) / 2.0f, static_cast<float>(kWindowHeight) - hudMargin});
		}

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
		// Update all enemies
		for (Enemy* enemy : enemies_) {
			if (enemy)
				enemy->Update();
		}

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

	// Draw all enemies
	for (Enemy* enemy : enemies_) {
		if (enemy)
			enemy->Draw();
	}

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

	// Draw HUD sprite in screen space after 3D post-draw
	if (hudSprite_) {
		// Sprite requires PreDraw/PostDraw when drawing; get command list and call
		KamataEngine::DirectXCommon* dx = KamataEngine::DirectXCommon::GetInstance();
		KamataEngine::Sprite::PreDraw(dx->GetCommandList());
		hudSprite_->Draw();
		KamataEngine::Sprite::PostDraw();
	}
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
	if (!player_ || enemies_.empty() || !player_->isAlive()) {
		return;
	}

	for (Enemy* enemy : enemies_) {
		if (!enemy || !enemy->isAlive())
			continue;

		// If player is attacking, check attack hitbox first
		if (player_->IsAttacking()) {
			AABB attackBox = player_->GetAttackAABB();
			if (IsCollisionAABBAABB(attackBox, enemy->GetAABB())) {
				// Hit enemy with attack
				enemy->OnCollision(player_);
				// Optionally, play hit effects here (camera shake, rumble)
				if (cameraController_) {
					cameraController_->StartShake(1.0f, 0.15f);
				}
				// Skip body collision for this enemy this frame
				continue;
			}
		}

		// Normal body collision (player takes damage)
		if (IsCollisionAABBAABB(player_->GetAABB(), enemy->GetAABB())) {
			player_->OnCollision(enemy);
			enemy->OnCollision(player_);
		}
	}

#pragma endregion
}

void GameScene::ChangePhase() {

	switch (phase_) {
	case Phase::kPlay:

		if (!player_->isAlive()) {
			phase_ = Phase::kDeath;
			const Vector3& deathPos = player_->GetPosition();

			if (deathParticle_) {
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

	// Delete existing enemies and recreate from map
	for (Enemy* enemy : enemies_) {
		delete enemy;
	}
	enemies_.clear();

	if (mapChipField_) {
		uint32_t vh = mapChipField_->GetNumBlockVertical();
		uint32_t wh = mapChipField_->GetNumBlockHorizontal();
		for (uint32_t y = 0; y < vh; ++y) {
			for (uint32_t x = 0; x < wh; ++x) {
				if (mapChipField_->GetMapChipTypeByIndex(x, y) == MapChipType::kEnemySpawn) {
					Vector3 enemyPosition = mapChipField_->GetMapChipPositionByIndex(x, y);
					Enemy* enemy = new Enemy();
					enemy->Initialize(&camera_, enemyPosition);
					enemies_.push_back(enemy);
				} else if (mapChipField_->GetMapChipTypeByIndex(x, y) == MapChipType::kEnemySpawnShield) {
					// We use CSV value '4' mapped to kEnemySpawnShield to indicate shield enemy spawn
					Vector3 enemyPosition = mapChipField_->GetMapChipPositionByIndex(x, y);
					FrontShieldEnemy* fse = new FrontShieldEnemy();
					fse->Initialize(&camera_, enemyPosition);
					// optional: adjust threshold for variety
					fse->SetFrontDotThreshold(0.6f);
					enemies_.push_back(fse);
				}
			}
		}
	}

	if (deathParticle_) {
		delete deathParticle_;
		deathParticle_ = nullptr;
	}

	// Recreate HUD sprite (texture likely already loaded in TextureManager)
	if (hudSprite_) {
		delete hudSprite_;
		hudSprite_ = nullptr;
	}
	if (hudTextureHandle_ != 0u) {
		const float hudWidth = 250.0f;
		const float hudHeight = 30.0f;
		const float hudMargin = 20.0f;
		hudSprite_ = KamataEngine::Sprite::Create(hudTextureHandle_, KamataEngine::Vector2{static_cast<float>(kWindowWidth) / 2.0f, static_cast<float>(kWindowHeight) - hudMargin}, KamataEngine::Vector4{1,1,1,1}, KamataEngine::Vector2{0.5f, 1.0f});
		if (hudSprite_) {
			hudSprite_->SetSize(KamataEngine::Vector2{hudWidth, hudHeight});
		}
	}

	GenerateBlocks();

	if (cameraController_) {
		cameraController_->SetTarget(player_);
		cameraController_->Reset();
	}

	phase_ = Phase::kPlay;
}
