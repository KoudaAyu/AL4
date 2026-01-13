#include "GameScene.h"

using namespace KamataEngine;

GameScene::GameScene() {}

GameScene::~GameScene() { delete model_; }

void GameScene::Initialize() {
	worldTransform_.Initialize();

	camera_.Initialize();

	model_ = Model::Create();
}

void GameScene::Update() {}

void GameScene::Draw() {
	Model::PreDraw();

	model_->Draw(worldTransform_, camera_);

	Model::PostDraw();
}