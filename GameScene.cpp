#include "GameScene.h"
#include "AABB.h"
#include <algorithm>
#include <random>

using namespace KamataEngine;

GameScene::GameScene() {}

GameScene::~GameScene() {
#ifdef _DEBUG
	delete debugCamera_;
#endif //  _DEBUG

	for (auto* apple : apples_) {
		delete apple;
	}
	for (auto* bomb : bombs_) {
		delete bomb;
	}
	delete model_;
	delete player_;
	for (auto& row : WorldTransformWalls_) {
		for (auto* wt : row) {
			delete wt;
		}
	}
	WorldTransformWalls_.clear();
	delete mapChipField_;
}

void GameScene::Initialize() {

#ifdef _DEBUG

	// デバックカメラの生成
	debugCamera_ = new KamataEngine::DebugCamera(kWindowWidth, kWindowHeight);

#endif //  _DEBUG

	model_ = Model::Create();
	textureHandle_ = TextureManager::Load("uvChecker.png");

#ifdef _DEBUG
	assert(textureHandle_);
#endif
	camera_.Initialize();

	mapChipField_ = new MapChipField();
	mapChipField_->Initialize();
	mapChipField_->LoadmapChipCsv("Resources/Block.csv");
	GenerateWalls();

	player_ = new Player();
	bool playerSpawned = false;

	for (uint32_t y = 0; y < mapChipField_->GetNumBlockVirtical(); ++y) {
		for (uint32_t x = 0; x < mapChipField_->GetNumBlockHorizontal(); ++x) {
			MapChipType type = mapChipField_->GetMapChipTypeByIndex(x, y);
			KamataEngine::Vector3 pos = mapChipField_->GetMapChipPositionByIndex(x, y);

			if (type == MapChipType::kPlayerSpawn) {
				player_->Initialize(model_, &camera_, pos);
				playerSpawned = true;
			} else if (type == MapChipType::kAppleSpawn) {

			} else if (type == MapChipType::kBombSpawn) {
				Bomb* bomb = new Bomb();
				bomb->Initialize(model_, &camera_, pos);
				bombs_.push_back(bomb);
			}
		}
	}

	std::random_device rd;
	std::mt19937 gen(rd());
	// カメラの位置をグリッドインデックスに変換
	int camX = static_cast<int>(camera_.translation_.x / mapChipField_->GetBlockWidth());
	int camY = static_cast<int>(camera_.translation_.y / mapChipField_->GetBlockHeight());

	int minX = (std::max)(0, camX - 5);
	int maxX = (std::min)(static_cast<int>(mapChipField_->GetNumBlockHorizontal()) - 1, camX + 6);
	int minY = (std::max)(0, camY - 5);
	int maxY = (std::min)(static_cast<int>(mapChipField_->GetNumBlockVirtical()) - 1, camY + 5);

	std::uniform_int_distribution<int> distX(minX, maxX);
	std::uniform_int_distribution<int> distY(minY, maxY);

	for (int i = 0; i < 10; ++i) {
		int xIndex, yIndex;
		while (true) {
			xIndex = distX(gen);
			yIndex = distY(gen);
			if (mapChipField_->GetMapChipTypeByIndex(xIndex, yIndex) != MapChipType::kWall) {
				KamataEngine::Vector3 pos = mapChipField_->GetMapChipPositionByIndex(xIndex, yIndex);
				// プレイヤーのスポーン位置と一致しないか判定
				if (!(pos.x == player_->GetPosition().x && pos.y == player_->GetPosition().y && pos.z == player_->GetPosition().z)) {
					// 既存のAppleと被らないか判定
					bool overlap = false;
					for (const auto* apple : apples_) {
						const auto& aPos = apple->GetPosition();
						if (aPos.x == pos.x && aPos.y == pos.y && aPos.z == pos.z) {
							overlap = true;
							break;
						}
					}
					// 既存のBombとも被らないか判定
					for (const auto* bomb : bombs_) {
						const auto& bPos = bomb->GetPosition();
						if (bPos.x == pos.x && bPos.y == pos.y && bPos.z == pos.z) {
							overlap = true;
							break;
						}
					}
					if (!overlap) {
						Apple* apple = new Apple();
						apple->Initialize(model_, &camera_, pos);
						apples_.push_back(apple);
						break;
					}
				}
			}
		}
	}

#ifdef _DEBUG
	assert(playerSpawned && "マップにkPlayerSpawnがありません");

	assert(playerSpawned && "マップにkPlayerSpawnがありません");
#endif
}

void GameScene::Update() {

#ifdef _DEBUG

	ImGui::Begin("Window");
	const Vector3& pos = player_->GetPosition();
	ImGui::Text("Player Position: X=%.2f, Y=%.2f, Z=%.2f", pos.x, pos.y, pos.z);
	int appleIndex = 0;
	for (auto* apple : apples_) {
		if (!apple)
			continue;
		const Vector3& apos = apple->GetPosition();
		ImGui::Text("Apple[%d] Position: X=%.2f, Y=%.2f, Z=%.2f", appleIndex, apos.x, apos.y, apos.z);
		++appleIndex;
	}
	const auto& bodyParts = player_->GetBodyParts();
	for (size_t i = 0; i < bodyParts.size(); ++i) {
		const auto& v = bodyParts[i];
		ImGui::Text("Part %zu: (%.2f, %.2f, %.2f)", i, v.x, v.y, v.z);
	}
	float camPos[3] = {camera_.translation_.x, camera_.translation_.y, camera_.translation_.z};
	if (ImGui::DragFloat3("Camera Position", camPos, 0.1f)) {
		camera_.translation_.x = camPos[0];
		camera_.translation_.y = camPos[1];
		camera_.translation_.z = camPos[2];
		camera_.UpdateMatrix();
		camera_.TransferMatrix();
	}
	ImGui::End();

	debugCamera_->Update();

	AxisIndicator::GetInstance()->SetVisible(true);
	AxisIndicator::GetInstance()->SetTargetCamera(&debugCamera_->GetCamera());

#endif //  _DEBUG

	player_->Update();

	camera_.translation_ = {10, 8, -50};
	camera_.UpdateMatrix();
	camera_.TransferMatrix();

	for (auto* apple : apples_) {
		if (!apple)
			continue;

		if (!apple->IsActive()) {
			apple->Respawn(mapChipField_, *player_);
			apple->Update(); // 追加: 行列更新を即反映
			continue;
		}

		apple->Update();

		if (CheckCollision(player_->GetPosition(), apple->GetPosition(), 1.0f)) {
			player_->Grow();
			apple->Respawn(mapChipField_, *player_);
			apple->Update(); // 追加: 行列更新を即反映
			continue;
		}
	}

	for (auto it = bombs_.begin(); it != bombs_.end();) {
		Bomb* bomb = *it;
		if (!bomb) {
			++it;
			continue;
		}
		bomb->Update();
		if (CheckCollision(player_->GetPosition(), bomb->GetPosition(), 1.0f)) {
			// 爆弾を消す
			player_->EatBomb();
			delete bomb;
			it = bombs_.erase(it);
		} else {
			++it;
		}
	}

	// 壁の更新
	for (uint32_t i = 0; i < WorldTransformWalls_.size(); ++i) {
		for (uint32_t j = 0; j < WorldTransformWalls_[i].size(); ++j) {
			if (WorldTransformWalls_[i][j]) { // nullチェック追加
				WorldTransformWalls_[i][j]->matWorld_ = MakeAffineMatrix(WorldTransformWalls_[i][j]->scale_, WorldTransformWalls_[i][j]->rotation_, WorldTransformWalls_[i][j]->translation_);
				WorldTransformWalls_[i][j]->TransferMatrix();
			}
		}
	}

#ifdef NDEBUG
	const auto& bodyParts = player_->GetBodyParts();
#endif

	// Playerの頭と体がぶつかった時の判定
	const Vector3& headPos = player_->GetPosition();
	for (size_t i = 1; i < bodyParts.size(); ++i) { // 0は頭なので1から
		if (CheckCollision(headPos, bodyParts[i], 1.0f)) {
			// ここに当たり判定時の処理を書く
			player_->SetAlive(false);
			break;
		}
	}

	const AABB& playerAABB = player_->GetAABB(); // プレイヤーのAABB

	for (uint32_t y = 0; y < mapChipField_->GetNumBlockVirtical(); ++y) {
		for (uint32_t x = 0; x < mapChipField_->GetNumBlockHorizontal(); ++x) {
			MapChipType type = mapChipField_->GetMapChipTypeByIndex(x, y);
			if (static_cast<int>(type) == 1) { // マップチップ番号1
				Vector3 chipPos = mapChipField_->GetMapChipPositionByIndex(x, y);

				// マップチップのAABBを生成
				float width = mapChipField_->GetBlockWidth();
				float height = mapChipField_->GetBlockHeight();
				AABB WallAABB;
				WallAABB.min = {chipPos.x - width / 2.0f, chipPos.y - height / 2.0f, chipPos.z - width / 2.0f};
				WallAABB.max = {chipPos.x + width / 2.0f, chipPos.y + height / 2.0f, chipPos.z + width / 2.0f};

				// AABB同士の当たり判定
				if (IsCollisionAABBAABB(playerAABB, WallAABB)) {
					player_->SetAlive(false);
				}
			}
		}
	}
}

void GameScene::Draw() {

	Model::PreDraw();

	// 壁の描画
	for (uint32_t i = 0; i < WorldTransformWalls_.size(); ++i) {
		for (uint32_t j = 0; j < WorldTransformWalls_[i].size(); ++j) {
			if (WorldTransformWalls_[i][j]) { // nullチェックを追加
				model_->Draw(*WorldTransformWalls_[i][j], camera_);
			}
		}
	}

	player_->Draw();

	for (auto* apple : apples_) {
		if (apple)
			apple->Draw();
	}

	for (auto* bomb : bombs_) {
		if (bomb)
			bomb->Draw();
	}

	Model::PostDraw();
}

void GameScene::GenerateWalls() {
	uint32_t numBlockVirtical = mapChipField_->GetNumBlockVirtical();
	uint32_t numBlockHorizontal = mapChipField_->GetNumBlockHorizontal();

	// 要素数を変更する
	WorldTransformWalls_.resize(numBlockVirtical);
	for (uint32_t i = 0; i < numBlockVirtical; ++i) {
		WorldTransformWalls_[i].resize(numBlockHorizontal);
	}

	for (uint32_t i = 0; i < numBlockVirtical; ++i) {
		for (uint32_t j = 0; j < numBlockHorizontal; ++j) {
			MapChipType type = mapChipField_->GetMapChipTypeByIndex(j, i);
			if (type == MapChipType::kWall) {
				if (!WorldTransformWalls_[i][j]) {
					WorldTransform* wt = new WorldTransform();
					wt->Initialize();
					WorldTransformWalls_[i][j] = wt;
					WorldTransformWalls_[i][j]->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
				}
			}
		}
	}
}

bool GameScene::CheckCollision(const Vector3& a, const Vector3& b, float radius) {
	float dx = a.x - b.x;
	float dy = a.y - b.y;
	float distSq = dx * dx + dy * dy;
	return distSq < (radius * radius);
}

void GameScene::RespawnAppleRandom(Apple* apple) {
	if (!apple)
		return;
	apple->Respawn(mapChipField_, *player_);
}
