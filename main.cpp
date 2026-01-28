#include <Windows.h>

#include "KamataEngine.h"

#include "GameScene.h"
#include "TitleScene.h"
#include "SelectScene.h"
#include "GameOverScene.h"

using namespace KamataEngine;

TitleScene* titleScene = nullptr;
SelectScene* selectScene = nullptr;
GameScene* gameScene = nullptr;
GameOverScene* gameOverScene = nullptr;

// 現在のステージインデックスを保持
static int gCurrentStageIndex = 0;

enum class Scene {

	kUnknown = 0,
	kTitle,
	kSelect,
	kGame,
	kGameOver,
};

Scene scene = Scene::kUnknown;

void ChangeScene();

void UpdateScene();

void DrawScene();

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	DirectXCommon* dxCommom = DirectXCommon::GetInstance();

	// 先にエンジンを初期化してからシーンを初期化する
	KamataEngine::Initialize(L"LE2B_07_コウダ_アユ_ねこきり");

#ifdef _DEBUG
	ImGuiManager* imguiManager = ImGuiManager::GetInstance();
#endif //  _DEBUG
#ifdef _DEBUG
	// デバッグビルドではセレクトシーンから開始
	scene = Scene::kGame;
#else
	scene = Scene::kTitle;
#endif

	// 初期シーンに応じて生成
	if (scene == Scene::kTitle) {
		titleScene = new TitleScene();
		titleScene->Initialize();
	} else if (scene == Scene::kGame) {
		// 初期ゲームシーン開始時のステージを設定
		gCurrentStageIndex = 0;
		gameScene = new GameScene(gCurrentStageIndex);
		gameScene->Initialize();
	} else if (scene == Scene::kSelect) {
		selectScene = new SelectScene();
		selectScene->Initialize();
	} else if (scene == Scene::kGameOver) {
		gameOverScene = new GameOverScene();
		gameOverScene->Initialize();
	}

	XINPUT_STATE state;

	while (true) {
		if (KamataEngine::Update()) {
			break;
		}

		Input::GetInstance()->GetJoystickState(0, state);

#ifdef _DEBUG
		imguiManager->Begin();
#endif //  _DEBUG

		
		ChangeScene();
		UpdateScene();

#ifdef _DEBUG
		imguiManager->End();
#endif

		dxCommom->PreDraw();

		
		DrawScene();

#ifdef _DEBUG
		AxisIndicator::GetInstance()->Draw();
		imguiManager->Draw();
#endif 

		dxCommom->PostDraw();
	}

	delete titleScene;
	delete selectScene;
	delete gameScene;
	delete gameOverScene;

	KamataEngine::Finalize();

	return 0;
}

void ChangeScene() {

	switch (scene) {
	case Scene::kTitle:
		if (titleScene && titleScene->IsFinished()) {
			scene = Scene::kSelect;
			delete titleScene;
			titleScene = nullptr;
			selectScene = new SelectScene();
			selectScene->Initialize();
		}
		break;
	case Scene::kSelect:
		if (selectScene && selectScene->IsFinished()) {
			scene = Scene::kGame;
			int chosen = selectScene->GetChosenStageIndex();
			if (chosen < 0) chosen = 0;
			gCurrentStageIndex = chosen; // 選択されたステージを保持
			delete selectScene;
			selectScene = nullptr;
			gameScene = new GameScene(gCurrentStageIndex);
			gameScene->Initialize();
		}
		break;
	case Scene::kGame:
		if (gameScene) {
			if (gameScene->IsBackToSelectRequested()) {
			
				delete gameScene;
				gameScene = nullptr;
				scene = Scene::kSelect;
				selectScene = new SelectScene();
				selectScene->Initialize();
				
				selectScene->SuppressPlayerNextJump();
				return;
			}
			// プレイヤーが死亡したらゲームオーバーへ遷移
			if (gameScene->IsPlayerDead()) {
				scene = Scene::kGameOver;
				delete gameScene;
				gameScene = nullptr;
				gameOverScene = new GameOverScene();
				gameOverScene->Initialize();
				return;
			}

			// クリア完了 -> ステージセレクトへ遷移
			if (gameScene->IsFinished()) {
		
				delete gameScene;
				gameScene = nullptr;
				scene = Scene::kSelect;
				selectScene = new SelectScene();
				selectScene->Initialize();
			
				selectScene->SuppressPlayerNextJump();
			}
		}
		break;
	case Scene::kGameOver:
		if (gameOverScene && gameOverScene->IsFinished()) {
			
			GameOverScene::Result r = gameOverScene->GetResult();
			delete gameOverScene;
			gameOverScene = nullptr;
			if (r == GameOverScene::Result::kRetryGame) {
				
				scene = Scene::kGame;
				gameScene = new GameScene(gCurrentStageIndex);
				gameScene->Initialize();
			} else if (r == GameOverScene::Result::kBackSelect) {
				
				scene = Scene::kSelect;
				selectScene = new SelectScene();
				selectScene->Initialize();
			} else {
			
				scene = Scene::kTitle;
				titleScene = new TitleScene();
				titleScene->Initialize();
			}
		}
		break;
	}
}

void UpdateScene() {
	switch (scene) {
	case Scene::kTitle:
		if (titleScene) titleScene->Update();
		break;
	case Scene::kSelect:
		if (selectScene) selectScene->Update();
		break;
	case Scene::kGame:
		if (gameScene) gameScene->Update();
		break;
	case Scene::kGameOver:
		if (gameOverScene) gameOverScene->Update();
		break;
	}
}

void DrawScene() {

	switch (scene) {
	case Scene::kTitle:
		if (titleScene) titleScene->Draw();
		break;
	case Scene::kSelect:
		if (selectScene) selectScene->Draw();
		break;
	case Scene::kGame:
		if (gameScene) gameScene->Draw();
		break;
	case Scene::kGameOver:
		if (gameOverScene) gameOverScene->Draw();
		break;
	}
}
