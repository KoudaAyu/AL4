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

// 加算
inline constexpr Vector3 operator+(const Vector3& lhs, const Vector3& rhs) noexcept { return Vector3{lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z}; }
inline constexpr Vector3& operator+=(Vector3& lhs, const Vector3& rhs) noexcept {
	lhs.x += rhs.x;
	lhs.y += rhs.y;
	lhs.z += rhs.z;
	return lhs;
}

// 減算
inline constexpr Vector3 operator-(const Vector3& lhs, const Vector3& rhs) noexcept { return Vector3{lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z}; }
inline constexpr Vector3& operator-=(Vector3& lhs, const Vector3& rhs) noexcept {
	lhs.x -= rhs.x;
	lhs.y -= rhs.y;
	lhs.z -= rhs.z;
	return lhs;
}

// 単項マイナス
inline constexpr Vector3 operator-(const Vector3& v) noexcept { return Vector3{-v.x, -v.y, -v.z}; }

// スカラー乗算（左右どちらの順序も対応）
inline constexpr Vector3 operator*(const Vector3& v, float s) noexcept { return Vector3{v.x * s, v.y * s, v.z * s}; }
inline constexpr Vector3 operator*(float s, const Vector3& v) noexcept { return Vector3{v.x * s, v.y * s, v.z * s}; }
inline constexpr Vector3& operator*=(Vector3& v, float s) noexcept {
	v.x *= s;
	v.y *= s;
	v.z *= s;
	return v;
}

// スカラー除算
inline constexpr Vector3 operator/(const Vector3& v, float s) noexcept { return Vector3{v.x / s, v.y / s, v.z / s}; }
inline constexpr Vector3& operator/=(Vector3& v, float s) noexcept {
	v.x /= s;
	v.y /= s;
	v.z /= s;
	return v;
}

// 線形補間（Vector3）
inline constexpr Vector3 Lerp(const Vector3& a, const Vector3& b, float t) noexcept { return Vector3{a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t}; }

// Vector3 と float の加算（必要なら使用可能）
inline constexpr Vector3 operator+(const Vector3& v, float s) noexcept { return {v.x + s, v.y + s, v.z + s}; }
inline constexpr Vector3 operator+(float s, const Vector3& v) noexcept { return v + s; }
