#include "KeyInput.h"

KeyInput* KeyInput::instance_ = nullptr;

KeyInput* KeyInput::GetInstance() {
	if (!instance_) {
		instance_ = new KeyInput();
	}
	return instance_;
}

KamataEngine::Vector2 KeyInput::GetLStick() const {
	KamataEngine::Vector2 result{0.0f, 0.0f};

	// XInputの左スティック値を取得
	XINPUT_STATE state{};
	if (XInputGetState(0, &state) == ERROR_SUCCESS) {
		// デッドゾーン処理
		constexpr SHORT DEADZONE = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
		SHORT x = state.Gamepad.sThumbLX;
		SHORT y = state.Gamepad.sThumbLY;
		if (abs(x) > DEADZONE || abs(y) > DEADZONE) {
			// -32768～32767を-1.0～1.0に正規化
			result.x = static_cast<float>(x) / 32767.0f;
			result.y = static_cast<float>(y) / 32767.0f;
		}
	}

	return result;
}
