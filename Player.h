#pragma once
#define NOMINMAX
#include <Windows.h>
#include <deque>
#include"KamataEngine.h"

enum class LRDirection
{ Left, Right };
enum class UDDirection 
{ Up, Down };

class Player
{
public:
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3 position);
	void Update();
	void Draw();

	 // 体を伸ばす
	void Grow();

	const KamataEngine::Vector3& GetPosition() const { return worldTransform_.translation_; }

	const std::vector<KamataEngine::Vector3>& GetBodyParts() const { return bodyParts_; }

private:

	KamataEngine::WorldTransform worldTransform_;

	KamataEngine::Model* model_ = nullptr;

	KamataEngine::Camera* camera_ = nullptr;

	uint32_t textureHandle_ = 0u;

	KamataEngine::Vector3 velocity_ = {};

	// 向き
	LRDirection lrDirection_ = LRDirection::Right;
	UDDirection udDirection_ = UDDirection::Up; 

	bool lrKnown_ = false;
	bool udKnown_ = false;

	//慣性移動
	static inline const float kAcceleration = 0.05f;

	//速度減少率
	static inline const float kAttenuation = 0.1f;

	//速度制限
	static inline const float kLimitSpeed = 2.0f;

	//旋回開始時の角度
	float turnFirstRotationY_ = 0.0f;

	//旋回タイマー
	float turnTimer_ = 0.0f;

	//旋回にかかる時間<秒>
	static inline const float kTimeTurn = 0.3f;

	//体の増加部分
	std::vector < KamataEngine::Vector3 > bodyParts_;
	std::deque<KamataEngine::WorldTransform> bodyPartTransforms_;

	// 追従遅延フレーム数
	static constexpr size_t kFollowDelay = 60; // 例: 5フレーム遅れ

	// 頭の座標履歴
	std::deque<KamataEngine::Vector3> headHistory_;

	static constexpr float unitLength = 1.0f;
};

