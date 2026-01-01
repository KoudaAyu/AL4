#include "GameScene.h"
#include "AABB.h"
#include "FrontShieldEnemy.h"
#include "MapChipField.h"
#include "MathUtl.h"
#include <2d/Sprite.h>
#include "KeyInput.h"
#include "Spike.h"
#include <algorithm>
using namespace KamataEngine;

GameScene::GameScene() {}

GameScene::~GameScene() {
#ifdef _DEBUG
	delete debugCamera_;
#endif 

	delete blockModel_;
	delete cameraController_;
	delete mapChipField_;
	delete model_;

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


	if (hudSprite_) {
		delete hudSprite_;
		hudSprite_ = nullptr;
	}

	
	if (uiLeftSprite_) {
		delete uiLeftSprite_;
		uiLeftSprite_ = nullptr;
	}
	if (uiMidSprite_) {
		delete uiMidSprite_;
		uiMidSprite_ = nullptr;
	}
	if (uiRightSprite_) {
		delete uiRightSprite_;
		uiRightSprite_ = nullptr;
	}

	
	if (countdownSprite_) {
		delete countdownSprite_;
		countdownSprite_ = nullptr;
	}

	// destroy heart sprites
	for (KamataEngine::Sprite* s : heartSprites_) {
		if (s) delete s;
	}
	heartSprites_.clear();

	for (Spike* s : spikes_) {
		delete s;
	}
	spikes_.clear();
}

void GameScene::Initialize() {

#ifdef _DEBUG

	// デバックカメラの生成
	debugCamera_ = new KamataEngine::DebugCamera(kWindowWidth, kWindowHeight);

#endif //  _DEBUG

	model_ = Model::Create();
	textureHandle_ = TextureManager::Load("uvChecker.png");

	// Use Block.obj for map chip blocks
	blockModel_ = Model::CreateFromOBJ("Block");
#ifdef _DEBUG
	assert(textureHandle_);
#endif
	camera_.Initialize();
	camera_.farZ = 3000.0f;
	camera_.UpdateProjectionMatrix();
	camera_.TransferMatrix();

	mapChipField_ = new MapChipField();
	mapChipField_->LoadMapChipCsv("Resources/Debug/Map/Block.csv");

	player_ = new Player();
	Vector3 playerPosition = {4.0f, 4.0f, 0.0f};
	if (mapChipField_) {
		uint32_t vh = mapChipField_->GetNumBlockVertical();
		uint32_t wh = mapChipField_->GetNumBlockHorizontal();
		bool found = false;
		for (uint32_t y = 0; y < vh && !found; ++y) {
			for (uint32_t x = 0; x < wh; ++x) {
				if (mapChipField_->GetMapChipTypeByIndex(x, y) == MapChipType::kReserved2) {
					playerPosition = mapChipField_->GetMapChipPositionByIndex(x, y);
					found = true;
					break;
				}
			}
		}
	}
	player_->Initialize(&camera_, playerPosition);
	player_->SetMapChipField(mapChipField_);


	heartTextureHandle_ = TextureManager::Load("Sprite/PlayerHP.png");
	if (heartTextureHandle_ != 0u && player_) {
	
		int hp = player_->GetHP();
		const float heartSize = 32.0f;
		const float heartMarginX = 20.0f;
		const float heartSpacing = 8.0f;
		for (int i = 0; i < hp; ++i) {
			float x = heartMarginX + i * (heartSize + heartSpacing);
			float y = static_cast<float>(kWindowHeight) - 20.0f;
			KamataEngine::Sprite* s = KamataEngine::Sprite::Create(heartTextureHandle_, KamataEngine::Vector2{x, y}, KamataEngine::Vector4{1,1,1,1}, KamataEngine::Vector2{0.0f, 1.0f});
			if (s) s->SetSize(KamataEngine::Vector2{heartSize, heartSize});
			heartSprites_.push_back(s);
		}
	}

	
	if (mapChipField_) {
		uint32_t vh = mapChipField_->GetNumBlockVertical();
		uint32_t wh = mapChipField_->GetNumBlockHorizontal();
		for (uint32_t y = 0; y < vh; ++y) {
			for (uint32_t x = 0; x < wh; ++x) {
				MapChipType t = mapChipField_->GetMapChipTypeByIndex(x, y);
				if (t == MapChipType::kEnemySpawn) {
					Vector3 enemyPosition = mapChipField_->GetMapChipPositionByIndex(x, y);
					Enemy* enemy = new Enemy();
					
					enemy->Initialize(&camera_, enemyPosition);
					enemies_.push_back(enemy);
				} else if (t == MapChipType::kEnemySpawnShield) {
					Vector3 enemyPosition = mapChipField_->GetMapChipPositionByIndex(x, y);
					FrontShieldEnemy* fse = new FrontShieldEnemy();
					fse->Initialize(&camera_, enemyPosition);
					fse->SetFrontDotThreshold(0.6f);
					enemies_.push_back(fse);
				} else if (t == MapChipType::kSpike) {
					Spike* s = new Spike();
					Vector3 pos = mapChipField_->GetMapChipPositionByIndex(x, y);
					s->SetPosition(pos);
					s->Initialize();
					spikes_.push_back(s);
				}
			}
		}
	}

	
	if (!spikes_.empty()) {
		DebugText::GetInstance()->ConsolePrintf("GameScene: created %u spikes\n", static_cast<uint32_t>(spikes_.size()));
		for (uint32_t i = 0; i < spikes_.size(); ++i) {
			Spike* s = spikes_[i];
			if (s) {
				DebugText::GetInstance()->ConsolePrintf("  spike[%u] HasModel=%s pos=(%.2f,%.2f,%.2f)\n", i, s->HasModel() ? "true" : "false", s->GetPosition().x, s->GetPosition().y, s->GetPosition().z);
			} else {
				DebugText::GetInstance()->ConsolePrintf("  spike[%u] is null\n", i);
			}
		}
	}

	if (mapChipField_) {
		uint32_t vh = mapChipField_->GetNumBlockVertical();
		uint32_t wh = mapChipField_->GetNumBlockHorizontal();
		for (uint32_t y = 0; y < vh; ++y) {
			for (uint32_t x = 0; x < wh; ++x) {
				MapChipType t = mapChipField_->GetMapChipTypeByIndex(x, y);
				if (t == MapChipType::kEnemySpawn) {
					Vector3 enemyPosition = mapChipField_->GetMapChipPositionByIndex(x, y);
					Enemy* enemy = new Enemy();
					
					enemy->Initialize(&camera_, enemyPosition);
					enemies_.push_back(enemy);
				} else if (t == MapChipType::kEnemySpawnShield) {
				
					Vector3 enemyPosition = mapChipField_->GetMapChipPositionByIndex(x, y);
					FrontShieldEnemy* fse = new FrontShieldEnemy();
					
					fse->Initialize(&camera_, enemyPosition);
					
					fse->SetFrontDotThreshold(0.6f);
					enemies_.push_back(fse);
				}
			}
		}
	}

	cameraController_ = new CameraController();
	
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
	
	readyForGameOver_ = false;

	hudTextureHandle_ = TextureManager::Load("Debug/Goal.png");
	if (hudTextureHandle_ != 0u) {
		
		const float hudWidth = 250.0f;
		const float hudHeight = 30.0f;
		const float hudMargin = 20.0f;
		
		hudSprite_ = KamataEngine::Sprite::Create(hudTextureHandle_, KamataEngine::Vector2{static_cast<float>(kWindowWidth) / 2.0f, hudMargin}, KamataEngine::Vector4{1,1,1,1}, KamataEngine::Vector2{0.5f, 0.0f});
		if (hudSprite_) {
			hudSprite_->SetSize(KamataEngine::Vector2{hudWidth, hudHeight});
		}
	}

	
	uiLeftTextureHandle_ = TextureManager::Load("Debug/Attack.png");
	if (uiLeftTextureHandle_ != 0u) {
		
		uiLeftSprite_ = KamataEngine::Sprite::Create(uiLeftTextureHandle_, KamataEngine::Vector2{50.0f, static_cast<float>(kWindowHeight) - 50.0f}, KamataEngine::Vector4{1,1,1,1}, KamataEngine::Vector2{0.0f, 1.0f});
		if (uiLeftSprite_) {
			uiLeftSprite_->SetSize(KamataEngine::Vector2{150.0f, 30.0f});
		}
	}
	
	uiMidTextureHandle_ = TextureManager::Load("Debug/Reset.png");
	if (uiMidTextureHandle_ != 0u) {
		uiMidSprite_ = KamataEngine::Sprite::Create(uiMidTextureHandle_, KamataEngine::Vector2{static_cast<float>(kWindowWidth) / 2.0f, static_cast<float>(kWindowHeight) - 50.0f}, KamataEngine::Vector4{1,1,1,1}, KamataEngine::Vector2{0.5f, 1.0f});
		if (uiMidSprite_) {
			uiMidSprite_->SetSize(KamataEngine::Vector2{200.0f, 30.0f});
		}
	}
	uiRightTextureHandle_ = TextureManager::Load("Debug/Jump.png");
	if (uiRightTextureHandle_ != 0u) {
		
		uiRightSprite_ = KamataEngine::Sprite::Create(uiRightTextureHandle_, KamataEngine::Vector2{static_cast<float>(kWindowWidth) - 50.0f, static_cast<float>(kWindowHeight) - 50.0f}, KamataEngine::Vector4{1,1,1,1}, KamataEngine::Vector2{1.0f, 1.0f});
		if (uiRightSprite_) {
			uiRightSprite_->SetSize(KamataEngine::Vector2{330.0f, 30.0f});
		}
	}


	victoryTimer_ = 0.0f;

	
	countdownTime_ = countdownStart_;

	for (int i = 0; i < 10; ++i) {
		char buf[64];
		sprintf_s(buf, "Number/%d.png", i);
		countdownTextureHandles_[i] = TextureManager::Load(buf);
	}

	int initialIndex = static_cast<int>(std::ceil(countdownTime_));
	if (initialIndex < 0) initialIndex = 0;
	if (initialIndex > 9) initialIndex = 9;
	if (countdownTextureHandles_[initialIndex] != 0u) {
		countdownSprite_ = KamataEngine::Sprite::Create(countdownTextureHandles_[initialIndex], KamataEngine::Vector2{static_cast<float>(kWindowWidth) / 2.0f, static_cast<float>(kWindowHeight) / 2.0f}, KamataEngine::Vector4{1,1,1,1}, KamataEngine::Vector2{0.5f, 0.5f});
		if (countdownSprite_) {
			
			countdownSprite_->SetSize(KamataEngine::Vector2{200.0f, 200.0f});
		}
	}


	phase_ = Phase::kCountdown;
}

void GameScene::Update() {

	// リセットキーでシーンをリセット
	if (Input::GetInstance()->TriggerKey(DIK_R) || KeyInput::GetInstance()->TriggerPadButton(XINPUT_GAMEPAD_B)) {
		Reset();
		return;
	}

	switch (phase_) {
	case Phase::kCountdown: {
		
		#ifdef _DEBUG
		
		if (Input::GetInstance()->TriggerKey(DIK_C)) {
			isDebugCameraActive_ = !isDebugCameraActive_;
		}
		if (isDebugCameraActive_) {
			debugCamera_->Update();
			camera_.matView = debugCamera_->GetCamera().matView;
			camera_.matProjection = debugCamera_->GetCamera().matProjection;
			camera_.TransferMatrix();
		} else {
			cameraController_->Update();
			camera_.UpdateMatrix();
		}
		#else
		cameraController_->Update();
		camera_.UpdateMatrix();
		#endif

		skydome_->Update();

		
		for (auto& row : worldTransformBlocks_) {
			for (WorldTransform* wt : row) {
				if (!wt) continue;
				wt->matWorld_ = MakeAffineMatrix(wt->scale_, wt->rotation_, wt->translation_);
				if (wt->parent_) wt->matWorld_ = Multiply(wt->parent_->matWorld_, wt->matWorld_);
				wt->TransferMatrix();
			}
		}

		
		if (player_) {
			
			KamataEngine::WorldTransform& pwt = player_->GetWorldTransform();
			pwt.matWorld_ = MakeAffineMatrix(pwt.scale_, pwt.rotation_, pwt.translation_);
			pwt.TransferMatrix();
		}

		
		countdownTime_ -= 1.0f / 60.0f;
		if (countdownTime_ <= 0.0f) {
		
			phase_ = Phase::kPlay;
		
			return;
		} else {
			
			int display = static_cast<int>(std::ceil(countdownTime_));
			if (display < 0) display = 0;
			if (display > 9) display = 9;
			if (countdownTextureHandles_[display] != 0u) {
				if (!countdownSprite_) {
					countdownSprite_ = KamataEngine::Sprite::Create(countdownTextureHandles_[display], KamataEngine::Vector2{static_cast<float>(kWindowWidth) / 2.0f, static_cast<float>(kWindowHeight) / 2.0f}, KamataEngine::Vector4{1,1,1,1}, KamataEngine::Vector2{0.5f, 0.5f});
					if (countdownSprite_) countdownSprite_->SetSize(KamataEngine::Vector2{200.0f, 200.0f});
				} else {
					countdownSprite_->SetTextureHandle(countdownTextureHandles_[display]);
				}
			}
		}

		break;
	}
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

#endif

		skydome_->Update();

		
		for (Enemy* enemy : enemies_) {
			if (enemy)
				enemy->Update();
		}

		
		if (!spikes_.empty()) {
			for (Spike* s : spikes_) {
				if (s) s->Update(1.0f / 60.0f);
			}
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
#endif 

		CheckAllCollisions();

		
		{
			bool anyAlive = false;
			for (Enemy* enemy : enemies_) {
				if (enemy && enemy->isAlive()) {
					anyAlive = true;
					break;
				}
			}
			if (!anyAlive) {
				
				if (victoryTimer_ <= 0.0f) {
				
					victoryTimer_ = 0.6f;
				}

				
				if (!player_->IsAttacking()) {
					victoryTimer_ -= 1.0f / 60.0f;
					if (victoryTimer_ <= 0.0f) {
						finished_ = true;
						return;
					}
				}
			}
		}

	
		if (hudSprite_) {
			
			const float hudMargin = 20.0f;
			hudSprite_->SetPosition(KamataEngine::Vector2{static_cast<float>(kWindowWidth) / 2.0f, hudMargin});
		}
	
		if (uiLeftSprite_) {
			uiLeftSprite_->SetPosition(KamataEngine::Vector2{50.0f, static_cast<float>(kWindowHeight) - 50.0f});
		}
		if (uiMidSprite_) {
			uiMidSprite_->SetPosition(KamataEngine::Vector2{static_cast<float>(kWindowWidth) / 2.0f, static_cast<float>(kWindowHeight) - 50.0f});
		}
		if (uiRightSprite_) {
			uiRightSprite_->SetPosition(KamataEngine::Vector2{static_cast<float>(kWindowWidth) - 50.0f, static_cast<float>(kWindowHeight) - 50.0f});
		}

		// Update heart sprites to match player's current HP
		{
			if (player_) {
				int hp = player_->GetHP();
				if (hp < 0) hp = 0;
				// If number of heart sprites differs from current HP, rebuild
				if (static_cast<int>(heartSprites_.size()) != hp) {
					// destroy existing
					for (KamataEngine::Sprite* s : heartSprites_) {
						if (s) delete s;
					}
					heartSprites_.clear();

					if (heartTextureHandle_ != 0u) {
						const float heartSize = 32.0f;
						const float heartMarginX = 20.0f;
						const float heartSpacing = 8.0f;
						for (int i = 0; i < hp; ++i) {
							float x = heartMarginX + i * (heartSize + heartSpacing);
							float y = static_cast<float>(kWindowHeight) - 20.0f; // bottom-aligned
							KamataEngine::Sprite* s = KamataEngine::Sprite::Create(heartTextureHandle_, KamataEngine::Vector2{x, y}, KamataEngine::Vector4{1,1,1,1}, KamataEngine::Vector2{0.0f, 1.0f});
							if (s) s->SetSize(KamataEngine::Vector2{heartSize, heartSize});
							heartSprites_.push_back(s);
						}
					}
				}
			}
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

#endif 

		skydome_->Update();
		// Update all enemies
		for (Enemy* enemy : enemies_) {
			if (enemy)
				enemy->Update();
		}

		// Particle関係
		if (phase_ == Phase::kDeath && deathParticle_) {
			deathParticle_->Update();
			
			if (deathParticle_->IsFinished()) {
				readyForGameOver_ = true;
			}
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
	if (phase_ != Phase::kCountdown) {
		for (Enemy* enemy : enemies_) {
			if (enemy && enemy->isAlive())
				enemy->Draw();
		}
	}



	if (!spikes_.empty()) {
		for (Spike* s : spikes_) {
			if (s) s->Draw(&camera_);
		}
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
	if (phase_ == Phase::kDeath && deathParticle_) {
		deathParticle_->Draw();
	}

	Model::PostDraw();

	
	if (hudSprite_ || uiLeftSprite_ || uiMidSprite_ || uiRightSprite_ || countdownSprite_ || !heartSprites_.empty()) {
		KamataEngine::DirectXCommon* dx = KamataEngine::DirectXCommon::GetInstance();
		KamataEngine::Sprite::PreDraw(dx->GetCommandList());
		if (hudSprite_) hudSprite_->Draw();
		if (uiLeftSprite_) uiLeftSprite_->Draw();
		if (uiMidSprite_) uiMidSprite_->Draw();
		if (uiRightSprite_) uiRightSprite_->Draw();
		if (phase_ == Phase::kCountdown && countdownSprite_) countdownSprite_->Draw();
		
		for (KamataEngine::Sprite* s : heartSprites_) {
			if (s) s->Draw();
		}
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
    if (!player_ || !player_->isAlive()) {
        return;
    }

    for (Enemy* enemy : enemies_) {
        if (!enemy || !enemy->isAlive())
            continue;

        if (player_->IsAttacking() && player_->IsAttackActive()) {
            AABB attackBox = player_->GetAttackAABB();
            if (IsCollisionAABB2D(attackBox, enemy->GetAABB())) {
                enemy->OnCollision(player_);
                if (cameraController_ && !player_->IsDying()) cameraController_->StartShake(1.0f, 0.15f);
                continue;
            }
        }

        if (IsCollisionAABB2D(player_->GetAABB(), enemy->GetAABB())) {
            player_->OnCollision(enemy);
            enemy->OnCollision(player_);
        }
    }

    // Spike とプレイヤーの当たり判定（Spike 側で AABB を提供）
    for (Spike* s : spikes_) {
        if (!s) continue;
        AABB pA = player_->GetAABB();
        AABB sA = s->GetAABB();
        if (IsCollisionAABB2D(pA, sA)) {
            
            float overlapX = std::min<float>(pA.max.x, sA.max.x) - std::max<float>(pA.min.x, sA.min.x);
            float overlapY = std::min<float>(pA.max.y, sA.max.y) - std::max<float>(pA.min.y, sA.min.y);

            
            Vector3 pCenter = {(pA.min.x + pA.max.x) * 0.5f, (pA.min.y + pA.max.y) * 0.5f, 0.0f};
            Vector3 sCenter = {(sA.min.x + sA.max.x) * 0.5f, (sA.min.y + sA.max.y) * 0.5f, 0.0f};

            
            KamataEngine::WorldTransform& pwt = player_->GetWorldTransform();
            if (overlapX < overlapY) {
                float dir = (pCenter.x < sCenter.x) ? -1.0f : 1.0f;
                pwt.translation_.x += dir * overlapX;
                player_->velocity_.x = 0.0f;
            } else {
                float dir = (pCenter.y < sCenter.y) ? -1.0f : 1.0f;
                pwt.translation_.y += dir * overlapY;
                player_->velocity_.y = 0.0f;
            }


            player_->UpdateAABB();

            
            player_->OnCollision(nullptr);
            if (cameraController_ && !player_->IsDying()) cameraController_->StartShake(1.5f, 0.3f);
            break;
        }
    }
}

void GameScene::ChangePhase() {

	switch (phase_) {
	case Phase::kPlay:

		if (!player_->isAlive()) {
		
			phase_ = Phase::kDeath;
			const Vector3 deathPos = player_->GetPosition();

			if (deathParticle_) {
				delete deathParticle_;
				deathParticle_ = nullptr;
			}

			deathParticle_ = new DeathParticle();
			deathParticle_->Initialize(model_, &camera_, deathPos);
			
			readyForGameOver_ = false;
		}

		break;
	case Phase::kDeath:

		break;
	case Phase::kCountdown:
		
		break;
	}
}

// リセット処理
void GameScene::Reset() {
	
	if (player_) {
		delete player_;
		player_ = nullptr;
	}
	Vector3 playerPosition = {4.0f, 4.0f, 0.0f};
	// Recompute spawn position from map chip 2 if available
	if (mapChipField_) {
		uint32_t vh = mapChipField_->GetNumBlockVertical();
		uint32_t wh = mapChipField_->GetNumBlockHorizontal();
		bool found = false;
		for (uint32_t y = 0; y < vh && !found; ++y) {
			for (uint32_t x = 0; x < wh; ++x) {
				if (mapChipField_->GetMapChipTypeByIndex(x, y) == MapChipType::kReserved2) {
					playerPosition = mapChipField_->GetMapChipPositionByIndex(x, y);
					found = true;
					break;
				}
			}
		}
	}
	player_ = new Player();
	player_->Initialize(&camera_, playerPosition);
	player_->SetMapChipField(mapChipField_);
	// 再生成したプレイヤーにもカメラコントローラを渡す
	if (cameraController_) {
		player_->SetCameraController(cameraController_);
	}

	
	for (Enemy* enemy : enemies_) {
		delete enemy;
	}
	enemies_.clear();

	
	for (Spike* s : spikes_) {
		delete s;
	}
	spikes_.clear();

	if (mapChipField_) {
		uint32_t vh = mapChipField_->GetNumBlockVertical();
		uint32_t wh = mapChipField_->GetNumBlockHorizontal();
		for (uint32_t y = 0; y < vh; ++y) {
			for (uint32_t x = 0; x < wh; ++x) {
				MapChipType t = mapChipField_->GetMapChipTypeByIndex(x, y);
				if (t == MapChipType::kEnemySpawn) {
					Vector3 enemyPosition = mapChipField_->GetMapChipPositionByIndex(x, y);
					Enemy* enemy = new Enemy();
					enemy->Initialize(&camera_, enemyPosition);
					enemies_.push_back(enemy);
				} else if (t == MapChipType::kEnemySpawnShield) {
					Vector3 enemyPosition = mapChipField_->GetMapChipPositionByIndex(x, y);
					FrontShieldEnemy* fse = new FrontShieldEnemy();
					fse->Initialize(&camera_, enemyPosition);
					fse->SetFrontDotThreshold(0.6f);
					enemies_.push_back(fse);
				} else if (t == MapChipType::kSpike) {
					Spike* s = new Spike();
					Vector3 pos = mapChipField_->GetMapChipPositionByIndex(x, y);
					s->SetPosition(pos);
					s->Initialize();
					spikes_.push_back(s);
				}
			}
		}
	}


	if (deathParticle_) {
		delete deathParticle_;
		deathParticle_ = nullptr;
	}

	
	if (hudSprite_) {
		delete hudSprite_;
		hudSprite_ = nullptr;
	}
	if (hudTextureHandle_ != 0u) {
		const float hudWidth = 250.0f;
		const float hudHeight = 30.0f;
		const float hudMargin = 20.0f;
		
		hudSprite_ = KamataEngine::Sprite::Create(hudTextureHandle_, KamataEngine::Vector2{static_cast<float>(kWindowWidth) / 2.0f, hudMargin}, KamataEngine::Vector4{1,1,1,1}, KamataEngine::Vector2{0.5f, 0.0f});
		if (hudSprite_) {
			hudSprite_->SetSize(KamataEngine::Vector2{hudWidth, hudHeight});
		}
	}

	
	if (uiLeftSprite_) {
		delete uiLeftSprite_;
		uiLeftSprite_ = nullptr;
	}
	if (uiMidSprite_) {
		delete uiMidSprite_;
		uiMidSprite_ = nullptr;
	}
	if (uiLeftTextureHandle_ != 0u) {
		
		uiLeftSprite_ = KamataEngine::Sprite::Create(uiLeftTextureHandle_, KamataEngine::Vector2{50.0f, static_cast<float>(kWindowHeight) - 50.0f}, KamataEngine::Vector4{1,1,1,1}, KamataEngine::Vector2{0.0f, 1.0f});
		if (uiLeftSprite_) uiLeftSprite_->SetSize(KamataEngine::Vector2{150.0f, 30.0f});
	}
	if (uiMidTextureHandle_ != 0u) {
		
		uiMidSprite_ = KamataEngine::Sprite::Create(uiMidTextureHandle_, KamataEngine::Vector2{static_cast<float>(kWindowWidth) / 2.0f, static_cast<float>(kWindowHeight) - 50.0f}, KamataEngine::Vector4{1,1,1,1}, KamataEngine::Vector2{0.5f, 1.0f});
		if (uiMidSprite_) uiMidSprite_->SetSize(KamataEngine::Vector2{150.0f, 30.0f});
	}
	if (uiRightSprite_) {
		delete uiRightSprite_;
		uiRightSprite_ = nullptr;
	}
	if (uiRightTextureHandle_ != 0u) {
		
		uiRightSprite_ = KamataEngine::Sprite::Create(uiRightTextureHandle_, KamataEngine::Vector2{static_cast<float>(kWindowWidth) - 50.0f, static_cast<float>(kWindowHeight) - 50.0f}, KamataEngine::Vector4{1,1,1,1}, KamataEngine::Vector2{1.0f, 1.0f});
		if (uiRightSprite_) uiRightSprite_->SetSize(KamataEngine::Vector2{330.0f, 30.0f});
	}

	GenerateBlocks();

	if (cameraController_) {
		cameraController_->SetTarget(player_);
		cameraController_->Reset();
	}

	
	phase_ = Phase::kCountdown;
	countdownTime_ = countdownStart_;

	victoryTimer_ = 0.0f;
}
