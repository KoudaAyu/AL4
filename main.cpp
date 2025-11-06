#include <Windows.h>

#include "KamataEngine.h"

#include "GameScene.h"
#include "TitleScene.h"

using namespace KamataEngine;

TitleScene* titleScene = nullptr;
GameScene* gameScene = nullptr;

enum class Scene {

	kUnknown = 0,
	kTitle,
	kGame,
};

Scene scene = Scene::kUnknown;

void ChangeScene();

void UpdateScene();

void DrawScene();

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	DirectXCommon* dxCommom = DirectXCommon::GetInstance();

	// 先にエンジンを初期化してからシーンを初期化する
	KamataEngine::Initialize(L"LE2B_07_コウダ_アユ");

#ifdef _DEBUG
	ImGuiManager* imguiManager = ImGuiManager::GetInstance();
#endif //  _DEBUG
#ifdef _DEBUG
	scene = Scene::kGame;
#else
	scene = Scene::kTitle;
#endif

	titleScene = new TitleScene();
	titleScene->Initialize();

	gameScene = new GameScene();
	gameScene->Initialize();

	while (true) {
		if (KamataEngine::Update()) {
			break;
		}

#ifdef _DEBUG

		imguiManager->Begin();

#endif //  _DEBUG
#ifdef _DEBUG
		gameScene->Update();
#else
		ChangeScene();
		UpdateScene();
#endif

#ifdef _DEBUG
		imguiManager->End();
#endif //  _DEBUG

		dxCommom->PreDraw();

#ifdef _DEBUG
		gameScene->Draw();
#else
		DrawScene();
#endif

#ifdef _DEBUG

		AxisIndicator::GetInstance()->Draw();

		imguiManager->Draw();
#endif //  _DEBUG

		dxCommom->PostDraw();
	}

	delete titleScene;
	delete gameScene;

	KamataEngine::Finalize();

	return 0;
}

void ChangeScene() {

	switch (scene) {
	case Scene::kTitle:
		if (titleScene->IsFinished()) {
			scene = Scene::kGame;
			delete titleScene;
			titleScene = nullptr;
			gameScene = new GameScene();
			gameScene->Initialize();
		}
		break;
	case Scene::kGame:
		if (gameScene->IsFinished()) {
			scene = Scene::kTitle;
			delete gameScene;
			gameScene = nullptr;
			titleScene = new TitleScene();
			titleScene->Initialize();
		}
		break;
	}
}

void UpdateScene() {
	switch (scene) {
	case Scene::kTitle:
		titleScene->Update();
		break;
	case Scene::kGame:
		gameScene->Update();
		break;
	}
}

void DrawScene() {

	switch (scene) {
	case Scene::kTitle:
		titleScene->Draw();
		break;

	case Scene::kGame:
		gameScene->Draw();
		break;
	}
}
