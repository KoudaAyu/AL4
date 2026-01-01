#include "AABB.h"

bool IsCollisionAABBAABB(const AABB& aabb1, const AABB& aabb2) {
	AABB fixedAABB1 = aabb1;

	// x軸のmin/maxを補正
	fixedAABB1.min.x = (std::min)(aabb1.min.x, aabb1.max.x);
	fixedAABB1.max.x = (std::max)(aabb1.min.x, aabb1.max.x);

	// y軸
	fixedAABB1.min.y = (std::min)(aabb1.min.y, aabb1.max.y);
	fixedAABB1.max.y = (std::max)(aabb1.min.y, aabb1.max.y);

	// z軸
	fixedAABB1.min.z = (std::min)(aabb1.min.z, aabb1.max.z);
	fixedAABB1.max.z = (std::max)(aabb1.min.z, aabb1.max.z);

	// 衝突判定
	if ((fixedAABB1.min.x <= aabb2.max.x && fixedAABB1.max.x >= aabb2.min.x) && (fixedAABB1.min.y <= aabb2.max.y && fixedAABB1.max.y >= aabb2.min.y) &&
	    (fixedAABB1.min.z <= aabb2.max.z && fixedAABB1.max.z >= aabb2.min.z)) {
		return true;
	}

	return false;
}

bool IsCollisionAABB2D(const AABB& aabb1, const AABB& aabb2) {
    // x/y のみで判定（Zは無視）
    float minX1 = (std::min)(aabb1.min.x, aabb1.max.x);
    float maxX1 = (std::max)(aabb1.min.x, aabb1.max.x);
    float minY1 = (std::min)(aabb1.min.y, aabb1.max.y);
    float maxY1 = (std::max)(aabb1.min.y, aabb1.max.y);

    float minX2 = (std::min)(aabb2.min.x, aabb2.max.x);
    float maxX2 = (std::max)(aabb2.min.x, aabb2.max.x);
    float minY2 = (std::min)(aabb2.min.y, aabb2.max.y);
    float maxY2 = (std::max)(aabb2.min.y, aabb2.max.y);

    return (minX1 <= maxX2 && maxX1 >= minX2) && (minY1 <= maxY2 && maxY1 >= minY2);
}