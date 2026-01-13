#pragma once

#include"KamataEngine.h"
#include"MathUtl.h"

class Player {
public:
	Player();
	~Player();
	void Initialize(KamataEngine::Camera* camera, const KamataEngine::Vector3 & pos);
	void Update();
	void Draw();

	void UpdateAABB();

	// AABB の取得
	const AABB& GetAABB() const;

	// 衝突時の処理
	void HandleCollision();

private:

	AABB aabb_{};

	KamataEngine::Vector3 velocity = {};

	static inline const float kAcceleration = 1.0f;

private:

	

	KamataEngine::Model* model_ = nullptr;

	KamataEngine::WorldTransform worldTransform_;

	KamataEngine::Camera* camera_ = nullptr;
};
