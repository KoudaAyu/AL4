#include "Apple.h"
#include "MathUtl.h"
#include"MapChipField.h"
#include"Player.h"
#include <random>
#include <vector>
#include <utility> 

using namespace KamataEngine;

void Apple::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position) {
	model_ = model;
	camera_ = camera;
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;

	textureHandle_ = TextureManager::Load("Red.png");
}

void Apple::Update() {
	// 必要ならアニメーションやロジック
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();
}

void Apple::Draw() {
	if (isActive_ && model_) {
		model_->Draw(worldTransform_, *camera_,textureHandle_);
	}
}

void Apple::Respawn(MapChipField* map, const Player& player) {
	if (!map) {
		isActive_ = false;
		return;
	}

	const int w = static_cast<int>(map->GetNumBlockHorizontal());
	const int h = static_cast<int>(map->GetNumBlockVirtical());

	std::vector<std::pair<int, int>> candidates;
	candidates.reserve(static_cast<size_t>(w) * static_cast<size_t>(h));

	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			// 緩和: 壁以外を候補（IsMovable が厳しすぎるケース対策）
			if (map->GetMapChipTypeByIndex(x, y) == MapChipType::kWall)
				continue;
			if (player.IsOccupyingGrid(x, y))
				continue;
			candidates.emplace_back(x, y);
		}
	}

	// フォールバック: 候補0なら占有チェックを一旦外してでも復帰させる
	if (candidates.empty()) {
		for (int y = 0; y < h; ++y) {
			for (int x = 0; x < w; ++x) {
				if (map->GetMapChipTypeByIndex(x, y) == MapChipType::kWall)
					continue;
				candidates.emplace_back(x, y);
			}
		}
		if (candidates.empty()) {
			isActive_ = false;
			return;
		}
	}

	std::mt19937 rng{std::random_device{}()};
	std::uniform_int_distribution<size_t> dist(0, candidates.size() - 1);
	auto [gx, gy] = candidates[dist(rng)];

	worldTransform_.translation_ = map->GetMapChipPositionByIndex(static_cast<uint32_t>(gx), static_cast<uint32_t>(gy));
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();

	isActive_ = true;
}