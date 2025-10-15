#pragma once
#include"KamataEngine.h"

class KeyInput {
public:
	static KeyInput* GetInstance();
	KamataEngine::Vector2 GetLStick() const;

	private:
	static KeyInput* instance_;
};
