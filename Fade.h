#pragma once
#include"KamataEngine.h"

class Fade
{
public:

	enum class Status
	{
		None,
		FadeIn,
		FadeOut,
	};


public:
	void Initialize();
	void Update();
	void Draw();

	/// <summary>
	/// フェード開始
	/// </summary>
	/// <param name="status">フェードの状態</param>
	/// <param name="duration">フェードの持続時間</param>
	void Start(Status status,float duration);

private:
	Status status_ = Status::None;

	// フェードの持続時間
	float duration_ = 0.0f;
	//　継続時間カウンター
	float counter_ = 0.0f;


private:

	KamataEngine::Sprite* fadeSprite_ = nullptr;
	uint32_t fadeTextureHandle_ = 0u;

};