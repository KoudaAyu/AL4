#pragma once
#define NOMINMAX
#include <Windows.h>
#include"KamataEngine.h"

enum class LRDirection
{ Left, Right };

class MapChipField;

class Player {
public:
	struct CollisionMapInfo {
		bool IsCollisionCeiling = false;
		bool IsLanding = false;
		bool IsCollisionWall = false;
		KamataEngine::Vector3 movement = {0.0f, 0.0f, 0.0f};
	};

	enum Corner {
		kRightBottom, // 右下
		kLeftBottom,  // 左下
		kRightTop,    // 右上
		kLeftTop,     // 左上

		kNumCorners // 要素数
	};

public:
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3 position);
	void Update();
	void Draw();

	void Move();
	void Jump();

	void SetCamera(KamataEngine::Camera* camera) { camera_ = camera; }

	// 当たり判定処理

	/// <summary>
	/// 移動量を加味して衝突判定
	/// </summary>
	/// <param name="collisionMapInfo"></param>
	void CollisionMap(CollisionMapInfo& collisionMapInfo);
	void CollisionMapUp(CollisionMapInfo& collisionMapInfo);
	void CollisionMapDown(CollisionMapInfo& collisionMapInfo);
	void CollisionMapLeft(CollisionMapInfo& collisionMapInfo);
	void CollisionMapRight(CollisionMapInfo& collisionMapInfo);

	KamataEngine::Vector3 CornerPisition(const KamataEngine::Vector3& center, Corner corner);

	void Reflection(CollisionMapInfo& collisionMapInfo);

public:
	const KamataEngine::WorldTransform& GetWorldTransform() const { return worldTransform_; }
	const KamataEngine::Vector3& GetPosition() const { return worldTransform_.translation_; }
	const KamataEngine::Vector3& GetVelocity() const { return velocity_; }

	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }

private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;
	uint32_t textureHandle_ = 0u;
	KamataEngine::Vector3 velocity_ = {};
	LRDirection lrDirection_ = LRDirection::Right;
	static inline const float kAcceleration = 0.05f;
	static inline const float kAttenuation = 0.1f;
	static inline const float kLimitSpeed = 2.0f;
	float turnFirstRotationY_ = 0.0f;
	float turnTimer_ = 0.0f;
	static inline const float kTimeTurn = 0.3f;
	bool isJump_ = false;
	float jumpVelocity_ = 0.0f;
	static inline float kGravity = 0.2f;
	static inline float kJumpVelocity = 2.0f;
	int jumpCount_ = 0;
	static inline const int kMaxJumpCount = 2;

	// キャラクターの当たり判定サイズ
	static inline const float kWidth = 0.8f * 2.0f;
	static inline const float kHeight = 0.8f * 2.0f;

	// 追加: マップチップ参照
	MapChipField* mapChipField_ = nullptr;
	
	static inline const float kBlank = 0.05f; // ceiling separation reduced to avoid deep push-down
	static inline const float kGroundBlankDown = 0.05f; // floor浮き量調整用
	static inline const float kWallBlank = 0.05f; // 壁との最小距離
	static inline const float kGroundProbeEps = 0.02f; // 床検出のための微小サンプル距離
};

