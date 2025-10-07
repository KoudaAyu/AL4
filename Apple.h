#pragma once
#include"KamataEngine.h"
 #include <memory>

class Apple {
public:
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);
	void Update();
	void Draw();
	const KamataEngine::Vector3& GetPosition() const { return worldTransform_.translation_; }
	bool IsActive() const { return isActive_; }
	void SetActive(bool active) { isActive_ = active; }

private:
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;
	KamataEngine::WorldTransform worldTransform_;
	bool isActive_ = true;
};