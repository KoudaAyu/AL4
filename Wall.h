#pragma once
#include "KamataEngine.h"
#include "MathUtl.h"
// 血管(壁)
class Wall {
public:
	Wall();
	~Wall();
	void Initialize(KamataEngine::Camera* camera, const KamataEngine::Vector3& pos);
	void Update();
	void Draw();

	void UpdateAABB();

	// AABB の取得
	const AABB& GetAABB() const;

	// 位置の取得
	const KamataEngine::Vector3& GetPosition() const;

private:
	AABB aabb_{};

private:
	KamataEngine::Model* model_ = nullptr;

	KamataEngine::WorldTransform worldTransform_;

	KamataEngine::Camera* camera_ = nullptr;
};
