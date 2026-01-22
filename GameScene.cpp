#include "GameScene.h"
#include "Player.h"
#include "Random.h"
#include <cmath>
#include <numbers>

using namespace KamataEngine;

GameScene::GameScene() {}

GameScene::~GameScene() {
	
	while (!enemies_.empty()) {
		Enemy* e = enemies_.front();
		if (e) delete e;
		enemies_.pop_front();
	}
	
	while (!walls_.empty()) {
		Wall* w = walls_.front();
		if (w) delete w;
		walls_.pop_front();
	}

	delete player_;
	delete model_;
	delete healer_;
}

void GameScene::Initialize() {
	worldTransform_.Initialize();

	camera_.Initialize();


	Random::SeedEngine();

#pragma region Player初期化
	player_ = new Player();
	player_->Initialize(&camera_, Vector3{-5.0f, 0.0f, 0.0f});
#pragma endregion Player初期化

#pragma region Wall初期化

	
	
		const float radius = 20.0f; 
		const float twoPi = 2.0f * std::numbers::pi_v<float>;
		for (int32_t i = 0; i < kMaxWall_; ++i) {
			float angle = twoPi * static_cast<float>(i) / static_cast<float>(kMaxWall_);
			float x = radius * std::cos(angle);
			float y = radius * std::sin(angle);
			Wall* wall = new Wall();
			wall->Initialize(&camera_, Vector3{x, y, 0.0f});
			
			float rotZ = angle + std::numbers::pi_v<float> * 0.5f; 
			wall->SetRotation(Vector3{0.0f, 0.0f, rotZ});
			walls_.push_back(wall);
		}
	
#pragma endregion Wall初期化

#pragma region Enemy初期化
	for (int32_t i = 0; i < kMaxEnemy_; ++i) {
		Enemy* enemy = new Enemy();
	
		float x = Random::GeneratorFloat(-10.0f, 10.0f);
		float y = Random::GeneratorFloat(-10.0f, 10.0f);
		enemy->Initialize(&camera_, Vector3{x, y, 0.0f});
		enemies_.push_back(enemy);
	}
#pragma endregion Enemy初期化

	#pragma region HealerActor初期化

	for (int32_t i = 0; i < kMaxHealerActor_; ++i) {
		HealerActor* healerActor = new HealerActor();
		float x = Random::GeneratorFloat(-15.0f, 15.0f);
		float y = Random::GeneratorFloat(-15.0f, 15.0f);
		healerActor->Initialize(&camera_, Vector3{ x, y, 0.0f });
		healerActor_.push_back(healerActor);
	}

	#pragma endregion HealerActor初期化

	// GameScene用のModel
	model_ = Model::Create();

	// Healer 初期化
	healer_ = new Healer();
}

void GameScene::Update() {

	for (int32_t i = 0; i < kMaxWall_; ++i) {
		Wall* w = walls_.front();
		if (w) w->Update();
		walls_.push_back(walls_.front());
		walls_.pop_front();
	}

	for (int32_t i = 0; i < kMaxEnemy_; ++i) {
		Enemy* e = enemies_.front();
		if (e) e->Update(walls_, healerActor_);
		enemies_.push_back(enemies_.front());
		enemies_.pop_front();
	}

	for (int32_t i = 0; i < kMaxHealerActor_; ++i)
	{
		HealerActor* ha = healerActor_.front();
		if (ha)
			ha->Update();
		healerActor_.push_back(healerActor_.front());
		healerActor_.pop_front();
	}
	player_->Update();

	CollisionCheck();

	// Healer は壊れた順に修復を試みる
	if (healer_) healer_->Update(&camera_, walls_, healerActor_);
}

void GameScene::Draw() {

	// Model描画前処理
	Model::PreDraw();

	/*model_->Draw(worldTransform_, camera_);*/

	for (int32_t i = 0; i < kMaxWall_; ++i) {
		Wall* w = walls_.front();
		if (w) w->Draw();
		walls_.push_back(walls_.front());
		walls_.pop_front();
	}

	for (int32_t i = 0; i < kMaxEnemy_; ++i) {
		Enemy* e = enemies_.front();
		if (e) e->Draw();
		enemies_.push_back(enemies_.front());
		enemies_.pop_front();
	}

	for (int32_t i = 0; i < kMaxHealerActor_; ++i)
	{
		HealerActor* ha = healerActor_.front();
		if (ha)
			ha->Draw();
		healerActor_.push_back(healerActor_.front());
		healerActor_.pop_front();
	}

	player_->Draw();

	// Model描画後処理
	Model::PostDraw();
}

void GameScene::CollisionCheck() {
#pragma region AABB更新
	if (!player_)
		return;

	// プレイヤーと血管の AABB を更新
	player_->UpdateAABB();
	const AABB& aabbPlayer = player_->GetAABB();
#pragma endregion AABB更新

#pragma region PlayerとWallの衝突判定
	for (Wall* wall : walls_) {
		if (!wall)
			continue;
		wall->UpdateAABB();
		const AABB& aabbWall = wall->GetAABB();

		if (IsCollisionAABBAABB(aabbPlayer, aabbWall)) {
			player_->HandleCollision();
		}
	}
#pragma endregion PlayerとWallの衝突判定

#pragma region PlayerとEnemyの衝突判定
	for (Enemy* enemy : enemies_) {
		if (!enemy)
			continue;

		// 敵の AABB を更新して取得
		enemy->UpdateAABB();
		const AABB& aabbEnemy = enemy->GetAABB();

		// Player と Enemy の衝突判定
		if (IsCollisionAABBAABB(aabbPlayer, aabbEnemy)) {
			player_->HandleCollision();
			enemy->HandleCollision();
		}
	}
#pragma endregion PlayerとEnemyの衝突判定

	// Enemy と Wall の衝突判定（接触フレームを蓄積して HP を減らす）
	for (auto wallIt = walls_.begin(); wallIt != walls_.end(); ++wallIt) {
		Wall* wall = *wallIt;
		if (!wall) continue;

		wall->UpdateAABB();
		const AABB& aabbWall = wall->GetAABB();

		bool touched = false;

		for (auto enemyIt = enemies_.begin(); enemyIt != enemies_.end(); ++enemyIt) {
			Enemy* enemy = *enemyIt;
			if (!enemy || !enemy->IsAlive()) continue;

			enemy->UpdateAABB();
			const AABB& aabbEnemy = enemy->GetAABB();

			if (IsCollisionAABBAABB(aabbEnemy, aabbWall)) {
				touched = true;
				bool destroyed = wall->AccumulateContactFrame();
				if (destroyed) {
				
					for (auto eIt = enemies_.begin(); eIt != enemies_.end(); ++eIt) {
						Enemy* e = *eIt;
						if (!e || !e->IsAlive()) continue;
						e->UpdateAABB();
						if (IsCollisionAABBAABB(e->GetAABB(), aabbWall)) {
							e->Kill();
						}
					}

					// 壊された位置を Healer に通知
					if (healer_) healer_->NotifyWallDestroyed(wall->GetPosition(), wall->GetRotation());

					delete wall;
					*wallIt = nullptr;
					break; // この壁は破壊されたので次の壁へ
				}
			}
		}

		if (*wallIt != nullptr && !touched) {
			// 徐々に接触フレームを減らし、断続的な接触でもHPが減るようにする
			wall->DecayContactFrames();
		}
	}
}
