#pragma once
#include "KamataEngine.h"

class KeyInput {
public:
	static KeyInput* GetInstance();
	KamataEngine::Vector2 GetLStick() const;
	bool TriggerPadButton(int button) const;

	static constexpr int XINPUT_BUTTON_A = XINPUT_GAMEPAD_A;


private:
	static KeyInput* instance_;
};
