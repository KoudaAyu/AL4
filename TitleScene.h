#pragma once
#include "KamataEngine.h"

#include"Fade.h"

class TitleScene {
public:

	TitleScene() = default;
	~TitleScene();

	void Initialize();
	void Update();
	void Draw();

public:
	bool IsFinished() { return finished_; }

private:
	bool finished_ = false;

	private:
	Fade* fade_ = nullptr;


};