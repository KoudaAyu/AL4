#include <Windows.h>

#include"KamataEngine.h"
#include"GameScene.h"

using namespace KamataEngine;

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	
	KamataEngine::Initialize(L"TD2_3");

	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	GameScene* gameScene = new GameScene();

	gameScene->Initialize();

	while (true)
	{
		if (KamataEngine::Update())
		{
			break;
		}

		#pragma region 更新処理

		// ゲームシーンの更新

		gameScene->Update();

		#pragma endregion 更新処理

		#pragma region 描画処理
		// 描画前処理
		dxCommon->PreDraw();

		// ゲームシーンの描画
		gameScene->Draw();

		//描画終了
		dxCommon->PostDraw();

		#pragma endregion 描画処理
	}

	delete gameScene;
	gameScene = nullptr;

	KamataEngine::Finalize();

	return 0;
}
