#include "Fade.h"

#include <algorithm>

using namespace KamataEngine;

void Fade::Initialize() {

	fadeTextureHandle_ = TextureManager::Load("Debug/black.png");

	fadeSprite_ = Sprite::Create(fadeTextureHandle_, {0.0f, 0.0f});
	fadeSprite_->SetSize(Vector2(1280, 720));
	fadeSprite_->SetColor(Vector4(0.0f, 0.0f, 0.0f, 1.0f));
}

void Fade::Update() {

	switch (status_) {
	case Status::None:
		break;
	case Status::FadeIn:
		counter_ += 1.0f / 60.0f;
		// フェード継続時間に達したらフェード終了
		if (counter_ >= duration_) {
			counter_ = duration_;
		}
		// フェード持続時間に近づくほどアルファ値を小さくする（1→0）
		fadeSprite_->SetColor(Vector4(0, 0, 0, 1.0f - std::clamp(counter_ / duration_, 0.0f, 1.0f)));
		break;
	case Status::FadeOut:
		counter_ += 1.0f / 60.0f;
		// フェード継続時間に達したらフェード終了
		if (counter_ >= duration_) {
			counter_ = duration_;
		}
		// フェード持続時間に近づくほどアルファ値を大きくする
		fadeSprite_->SetColor(Vector4(0, 0, 0, std::clamp(counter_ / duration_, 0.0f, 1.0f)));
		break;
	}
}

void Fade::Draw() {

	Sprite::PreDraw();
	if (status_ == Status::None) {
		return;
	}
	fadeSprite_->Draw();
	Sprite::PostDraw();
}

void Fade::Start(Status status, float duration) {

	status_ = status;
	duration_ = duration;
	counter_ = 0.0f;
}

void Fade::Stop() { status_ = Status::None; }

bool Fade::IsFinished() const {

	switch (status_) {
	case Status::FadeIn:
	case Status::FadeOut:
		if (counter_ >= duration_) {
			return true;
		} else {
			return false;
		}
	}
	return true;
}
