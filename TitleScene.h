#pragma once
#include "KamataEngine.h"

class TitleScene {
public:
	void Initialize();
	void Update();
	void Draw();

public:
	bool IsFinished() { return finished_; }

private:
	bool finished_ = false;
};