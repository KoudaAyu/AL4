#pragma once
#include "KamataEngine.h"

using namespace KamataEngine;

Matrix4x4 Multiply(Matrix4x4 m1, Matrix4x4 m2);

Matrix4x4 MakeTranslateMatrix(const Vector3& translate);

Matrix4x4 MakeScaleMatrix(const Vector3& scale);

Matrix4x4 MakeRotateXMatrix(float radian);

Matrix4x4 MakeRotateYMatrix(float radian);

Matrix4x4 MakeRotateZMatrix(float radian);

Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);



// Vector3 同士の加算
inline constexpr Vector3 operator+(const Vector3& lhs, const Vector3& rhs) noexcept {
    return Vector3{lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
}

// 任意（あると便利）：複合代入
inline constexpr Vector3& operator+=(Vector3& lhs, const Vector3& rhs) noexcept {
    lhs.x += rhs.x;
    lhs.y += rhs.y;
    lhs.z += rhs.z;
    return lhs;
}

