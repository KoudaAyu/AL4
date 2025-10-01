#include <Windows.h>

#include"KamataEngine.h"

using namespace KamataEngine;

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	
	KamataEngine::Initialize(L"LE2B_07_コウダ_アユ");

	DirectXCommon* dxCommom = DirectXCommon::GetInstance();

	while (true)
	{
		if (KamataEngine::Update())
		{
			break;
		}

		dxCommom->PreDraw();

		dxCommom->PostDraw();
	}


	

	KamataEngine::Finalize();

	return 0;
}
