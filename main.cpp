#include <Windows.h>

#include "GameScene.h"
#include "KamataEngine.h"

using namespace KamataEngine;

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	DirectXCommon* dxCommom = DirectXCommon::GetInstance();

	GameScene* gameScene = new GameScene();

#ifdef _DEBUG
	ImGuiManager* imguiManager = ImGuiManager::GetInstance();

#endif //  _DEBUG

	KamataEngine::Initialize(L"LE2B_07_コウダ_アユ");

	gameScene->Initialize();

	while (true) {
		if (KamataEngine::Update()) {
			break;
		}

#ifdef _DEBUG

		imguiManager->Begin();

#endif //  _DEBUG

		gameScene->Update();

#ifdef _DEBUG
		imguiManager->End();
#endif //  _DEBUG

		dxCommom->PreDraw();

		gameScene->Draw();

#ifdef _DEBUG
		imguiManager->Draw();
#endif //  _DEBUG

		dxCommom->PostDraw();
	}

	delete gameScene;
	gameScene = nullptr;

	KamataEngine::Finalize();

	return 0;
}
