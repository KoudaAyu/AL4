#pragma once
#include "KamataEngine.h"

struct AABB {
	KamataEngine::Vector3 min; // 最小点
	KamataEngine::Vector3 max; // 最大点
};

bool IsCollisionAABBAABB(const AABB& aabb1, const AABB& aabb2);

// 2D用のAABB当たり判定（Z軸は無視）
bool IsCollisionAABB2D(const AABB& aabb1, const AABB& aabb2);