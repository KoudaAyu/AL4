#include "GameScene.h"
#include "Player.h"

using namespace KamataEngine;

GameScene::GameScene() {}

GameScene::~GameScene() { delete model_; }

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
	enemy_ = new Enemy();
	enemy_->Initialize(&camera_, Vector3{0.0f, -10.0f, 0.0f});
#pragma endregion Enemy初期化

	// GameScene用のModel
	model_ = Model::Create();
}

void GameScene::Update() {
	bloodVessels_->Update();
	enemy_->Update();
	player_->Update();

	CollisionCheck();
}

void GameScene::Draw() {

	// Model描画前処理
	Model::PreDraw();

	/*model_->Draw(worldTransform_, camera_);*/

	bloodVessels_->Draw();
	enemy_->Draw();
	player_->Draw();

	// Model描画後処理
	Model::PostDraw();
}

void GameScene::CollisionCheck() {
#pragma region AABB更新
	const AABB& aabbPlayer = player_->GetAABB();
	const AABB& aabbEnemy = enemy_->GetAABB();
	const AABB& aabbBlood = bloodVessels_->GetAABB();
#pragma endregion AABB更新

#pragma region PlayerとBloodVesselsの衝突判定

	if (IsCollisionAABBAABB(aabbPlayer, aabbBlood)) {
		player_->HandleCollision();
	}

#pragma endregion PlayerとBloodVesselsの衝突判定

#pragma region PlayerとEnemyの衝突判定
	if (IsCollisionAABBAABB(aabbPlayer, aabbEnemy)) {
		player_->HandleCollision();
		enemy_->HandleCollision();
	}
#pragma endregion PlayerとEnemyの衝突判定

#pragma endregion EnemyとBloodVesselsの衝突判定
	if (IsCollisionAABBAABB(aabbEnemy, aabbBlood)) {
		enemy_->HandleCollision();
	}
#pragma endregion EnemyとBloodVesselsの衝突判定
}
