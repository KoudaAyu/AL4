#include <Windows.h>

#include"KamataEngine.h"
#include"GameScene.h"

using namespace KamataEngine;

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	
	DirectXCommon* dxCommom = DirectXCommon::GetInstance();

	GameScene* gameScene = new GameScene();

	KamataEngine::Initialize(L"LE2B_07_コウダ_アユ");

	gameScene->Initialize();

	while (true)
	{
		if (KamataEngine::Update())
		{
			break;
		}

		gameScene->Update();

		dxCommom->PreDraw();

		gameScene->Draw();

		dxCommom->PostDraw();
	}

	delete gameScene;
	gameScene = nullptr;

	KamataEngine::Finalize();

	return 0;
}
