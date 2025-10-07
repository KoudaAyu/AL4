#include "GameScene.h"

using namespace KamataEngine;

GameScene::GameScene() {}

GameScene::~GameScene() {
#ifdef _DEBUG
	delete debugCamera_;
#endif //  _DEBUG

	delete model_;
	delete player_;
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

	  Apple* apple1 = new Apple();
	apple1->Initialize(model_, &camera_, {3, 5, 0});
	apples_.push_back(apple1);

	// 2つ目のりんごを追加
	Apple* apple2 = new Apple();
	apple2->Initialize(model_, &camera_, {-4, -2, 0}); // 好きな座標に
	apples_.push_back(apple2);

	player_ = new Player();
	Vector3 playerPosition = {0.0f, 0.0f, 0.0f};
	player_->Initialize(model_, &camera_, playerPosition);
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

	#ifdef NDEBUG
	const auto& bodyParts = player_->GetBodyParts();
	#endif

	//Playerの頭と体がぶつかった時の判定
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

	player_->Draw();

	for (auto* apple : apples_) {
		if (apple)
			apple->Draw();
	}

	Model::PostDraw();
}

bool GameScene::CheckCollision(const Vector3& a, const Vector3& b, float radius) {
	float dx = a.x - b.x;
	float dy = a.y - b.y;
	float dz = a.z - b.z;
	float distSq = dx * dx + dy * dy + dz * dz;
	return distSq < (radius * radius);
}
