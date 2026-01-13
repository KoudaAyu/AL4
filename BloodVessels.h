#pragma once
#include "KamataEngine.h"
#include "MathUtl.h"
// 血管(壁)
class BloodVessels {
public:
	BloodVessels();
	~BloodVessels();
	void Initialize(KamataEngine::Camera* camera, const KamataEngine::Vector3& pos);
	void Update();
	void Draw();

	void UpdateAABB();

	// AABB の取得
	const AABB& GetAABB() const;

private:
	AABB aabb_{};

private:
	KamataEngine::Model* model_ = nullptr;

	KamataEngine::WorldTransform worldTransform_;

	KamataEngine::Camera* camera_ = nullptr;
};
