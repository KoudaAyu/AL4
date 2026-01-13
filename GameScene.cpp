#include "GameScene.h"
#include "Player.h"

using namespace KamataEngine;

GameScene::GameScene() {}

GameScene::~GameScene() { 
	for (int32_t i = 0; i < kMaxEnemy_; ++i) {
		delete enemies_.front();
		enemies_.pop_front();
	}
	delete bloodVessels_;
	delete player_;
	delete model_;
}

void GameScene::Initialize() {
	worldTransform_.Initialize();

	camera_.Initialize();

#pragma region Player初期化
	player_ = new Player();
	player_->Initialize(&camera_, Vector3{0.0f, 0.0f, 0.0f});
#pragma endregion Player初期化

#pragma region BloodVessels初期化
	bloodVessels_ = new BloodVessels();
	bloodVessels_->Initialize(&camera_, Vector3{0.0f, 10.0f, 0.0f});
#pragma endregion BloodVessels初期化

#pragma region Enemy初期化
	for (int32_t i = 0; i < kMaxEnemy_; ++i) {
		Enemy* enemy = new Enemy();
		enemy->Initialize(&camera_, Vector3{static_cast<float>(i * 5 - 10), -10.0f, 0.0f});
		enemies_.push_back(enemy);
	}
#pragma endregion Enemy初期化

	// GameScene用のModel
	model_ = Model::Create();
}

void GameScene::Update() {
	bloodVessels_->Update();
	for (int32_t i = 0; i < kMaxEnemy_; ++i) {
		enemies_.front()->Update();
		enemies_.push_back(enemies_.front());
		enemies_.pop_front();
	}
	player_->Update();

	CollisionCheck();
}

void GameScene::Draw() {

	// Model描画前処理
	Model::PreDraw();

	/*model_->Draw(worldTransform_, camera_);*/

	bloodVessels_->Draw();
	for (int32_t i = 0; i < kMaxEnemy_; ++i) {
		enemies_.front()->Draw();
		enemies_.push_back(enemies_.front());
		enemies_.pop_front();
	}
	player_->Draw();

	// Model描画後処理
	Model::PostDraw();
}

void GameScene::CollisionCheck() {
#pragma region AABB更新
	if (!player_ || !bloodVessels_) return;

	// プレイヤーと血管の AABB を更新
	player_->UpdateAABB();
	bloodVessels_->UpdateAABB();

	const AABB& aabbPlayer = player_->GetAABB();
	const AABB& aabbBlood = bloodVessels_->GetAABB();
#pragma endregion AABB更新

#pragma region PlayerとBloodVesselsの衝突判定

	if (IsCollisionAABBAABB(aabbPlayer, aabbBlood)) {
		player_->HandleCollision();
	}

#pragma endregion PlayerとBloodVesselsの衝突判定

#pragma region PlayerとEnemyの衝突判定
	for (Enemy* enemy : enemies_) {
		if (!enemy) continue;

		// 敵の AABB を更新して取得
		enemy->UpdateAABB();
		const AABB& aabbEnemy = enemy->GetAABB();

		// Player と Enemy の衝突判定
		if (IsCollisionAABBAABB(aabbPlayer, aabbEnemy)) {
			player_->HandleCollision();
			enemy->HandleCollision();
		}
#pragma endregion PlayerとEnemyの衝突判定

#pragma endregion EnemyとBloodVesselsの衝突判定
		if (IsCollisionAABBAABB(aabbEnemy, aabbBlood)) {
			enemy->HandleCollision();
		}
	}
#pragma endregion EnemyとBloodVesselsの衝突判定
}
