#pragma once
#include"KamataEngine.h"
#include <cmath>
using namespace KamataEngine;

Matrix4x4 Multiply(Matrix4x4 m1, Matrix4x4 m2);

Matrix4x4 MakeTranslateMatrix(const Vector3& translate);

Matrix4x4 MakeScaleMatrix(const Vector3& scale);

Matrix4x4 MakeRotateXMatrix(float radian);

Matrix4x4 MakeRotateYMatrix(float radian);

Matrix4x4 MakeRotateZMatrix(float radian);

Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);

inline KamataEngine::Vector3& operator+=(KamataEngine::Vector3& lhs, const KamataEngine::Vector3& rhs) {
	lhs.x += rhs.x;
	lhs.y += rhs.y;
	lhs.z += rhs.z;
	return lhs;
}

inline Vector3 Normalize(const Vector3& v) {
	float len = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
	if (len == 0.0f)
		return {0, 0, 0};
	return {v.x / len, v.y / len, v.z / len};
}

inline float Length(const Vector3& v) {
	return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

inline KamataEngine::Vector3 operator+(const KamataEngine::Vector3& a, const KamataEngine::Vector3& b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }

// 単項マイナス
inline KamataEngine::Vector3 operator-(const KamataEngine::Vector3& v) { return {-v.x, -v.y, -v.z}; }

// スカラー倍（Vector3 * float）
inline KamataEngine::Vector3 operator*(const KamataEngine::Vector3& v, float s) { return {v.x * s, v.y * s, v.z * s}; }

// スカラー倍（float * Vector3）も必要なら
inline KamataEngine::Vector3 operator*(float s, const KamataEngine::Vector3& v) { return {v.x * s, v.y * s, v.z * s}; }