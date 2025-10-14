#include "GameScene.h"

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

	// りんごを追加
	Apple* apple1 = new Apple();
	apple1->Initialize(model_, &camera_, {3, 5, 0});
	apples_.push_back(apple1);

	// 2つ目のりんごを追加
	Apple* apple2 = new Apple();
	apple2->Initialize(model_, &camera_, {-4, -2, 0}); // 好きな座標に
	apples_.push_back(apple2);

	// 爆弾を追加
	Bomb* bomb1 = new Bomb();
	bomb1->Initialize(model_, &camera_, {0, 3, 0});
	bombs_.push_back(bomb1);

	Bomb* bomb2 = new Bomb();
	bomb2->Initialize(model_, &camera_, {5, -3, 0});
	bombs_.push_back(bomb2);

	player_ = new Player();
	Vector3 playerPosition = {0.0f, 0.0f, 0.0f};
	player_->Initialize(model_, &camera_, playerPosition);

	//// 要素数
	//const uint32_t kNumBlockVirtical = 10;
	//const uint32_t kNumBlockHorizontal = 20;
	//// ブロック一個分の横幅
	//const float kBlockWidth = 2.0f;
	//const float kBlockHeight = 2.0f;
	//// 要素数を変更する
	//WorldTransformWalls_.resize(kNumBlockHorizontal);
	//for (uint32_t i = 0; i < kNumBlockVirtical; ++i) {
	//	WorldTransformWalls_[i].resize(kNumBlockHorizontal);
	//}
	//// 壁の生成
	//for (uint32_t i = 0; i < kNumBlockVirtical; ++i) {
	//	for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {

	//		WorldTransformWalls_[i][j] = new WorldTransform();
	//		WorldTransformWalls_[i][j]->Initialize();
	//		WorldTransformWalls_[i][j]->translation_.x = kBlockWidth * j;
	//		WorldTransformWalls_[i][j]->translation_.y = kBlockHeight * i;
	//	}
	//}

	mapChipField_ = new MapChipField();
	mapChipField_->Initialize();
	mapChipField_->LoadmapChipCsv("Resources/Block.csv");
	GenerateWalls();
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
	ImGui::End();

	debugCamera_->Update();

	AxisIndicator::GetInstance()->SetVisible(true);
	AxisIndicator::GetInstance()->SetTargetCamera(&debugCamera_->GetCamera());

#endif //  _DEBUG

	player_->Update();

	for (auto* apple : apples_) {
		if (!apple)
			continue;
		apple->Update();
		if (apple->IsActive() && CheckCollision(player_->GetPosition(), apple->GetPosition(), 1.0f)) {
			apple->SetActive(false);
			player_->Grow();
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
				WorldTransformWalls_[i][j]->matWorld_ = MakeAffineMatrix
				(WorldTransformWalls_[i][j]->scale_, WorldTransformWalls_[i][j]->rotation_,
					WorldTransformWalls_[i][j]->translation_);
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
			break;
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
	float dz = a.z - b.z;
	float distSq = dx * dx + dy * dy + dz * dz;
	return distSq < (radius * radius);
}
