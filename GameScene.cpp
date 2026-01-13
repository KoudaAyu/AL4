#include "GameScene.h"
#include "Player.h"

using namespace KamataEngine;

GameScene::GameScene() {}

GameScene::~GameScene() {
	for (int32_t i = 0; i < kMaxEnemy_; ++i) {
		delete enemies_.front();
		enemies_.pop_front();
	}
	for (int32_t i = 0; i < kMaxWall_; ++i) {
		delete walls_.front();
		walls_.pop_front();
	}

	delete player_;
	delete model_;
}

void GameScene::Initialize() {
	worldTransform_.Initialize();

	camera_.Initialize();

#pragma region Player初期化
	player_ = new Player();
	player_->Initialize(&camera_, Vector3{-5.0f, 0.0f, 0.0f});
#pragma endregion Player初期化

#pragma region Wall初期化

	for (int32_t i = 0; i < kMaxWall_; ++i) {
		Wall* wall = new Wall();
		wall->Initialize(&camera_, Vector3{static_cast<float>(i * 5 ), static_cast<float>(i * 8), 0.0f});
		walls_.push_back(wall);
	}
#pragma endregion Wall初期化

#pragma region Enemy初期化
	for (int32_t i = 0; i < kMaxEnemy_; ++i) {
		Enemy* enemy = new Enemy();
		enemy->Initialize(&camera_, Vector3{static_cast<float>(i * 5 - 10), static_cast<float>(i * 10), 0.0f});
		enemies_.push_back(enemy);
	}
#pragma endregion Enemy初期化

	// GameScene用のModel
	model_ = Model::Create();
}

void GameScene::Update() {

	for (int32_t i = 0; i < kMaxWall_; ++i) {
		walls_.front()->Update();
		walls_.push_back(walls_.front());
		walls_.pop_front();
	}

	for (int32_t i = 0; i < kMaxEnemy_; ++i) {
		enemies_.front()->Update(walls_);
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

	for (int32_t i = 0; i < kMaxWall_; ++i) {
		walls_.front()->Draw();
		walls_.push_back(walls_.front());
		walls_.pop_front();
	}

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
#pragma endregion PlayerとEnemyの衝突判定

#pragma endregion EnemyとWallの衝突判定
		// Enemy と Wall の衝突判定（walls_ は list に対応）
		for (Wall* wall : walls_) {
			if (!wall)
				continue;
			const AABB& aabbWall = wall->GetAABB();
			if (IsCollisionAABBAABB(aabbEnemy, aabbWall)) {
				enemy->HandleCollision();
			}
		}
#pragma endregion EnemyとWallの衝突判定
	}
}
