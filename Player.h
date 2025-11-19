#pragma once
#define NOMINMAX
#include "KamataEngine.h"

#include <algorithm>
#include <assert.h>
#include <cstdint>
#include <numbers>

#include"MathUtl.h"

using namespace KamataEngine;

enum class WallSide {
	kNone,
	kLeft,
	kRight,
};

struct CollisionMapInfo {
	bool isCeilingCollision_ = false; // 天井衝突
	bool isLanding_ = false;          // 着地
	bool isWallContact_ = false;      // 壁接触
	Vector3 movement_;
	WallSide wallSide_ = WallSide::kNone; // どちらの壁に接触しているか
};

class MapChipField;

class Player {
public:
	enum class LRDirection {
		kRight,
		kLeft,
	};

	// 角
	enum Corner {
		kLeftTop,     // 左上
		kRightTop,    // 右上
		kLeftBottom,  // 左下
		kRightBottom, // 右下

		kNumCorners, // 要素数
	};

	// コンストラクタ
	Player();

	// デストラクタ
	~Player();

	// 初期化
	void Initialize(Model* model, Camera* camera, const Vector3& position);

	void HandleMovementInput();

	// 更新
	void Update();

	// 描画
	void Draw();

	Vector3 velocity_ = {};

	Vector3 CornerPosition(const Vector3& center, Corner corner);

	LRDirection lrDirection_ = LRDirection::kRight;

	const WorldTransform& GetTransform() const { return worldTransform_; }

	const Vector3& GetVelocity() const { return velocity_; }

	void SetMapChipField(MapChipField* mapChipField) { this->mapChipField_ = mapChipField; }

	void mapChipCollisionCheck(CollisionMapInfo& info);

	/// <summary>
	/// 判定結果を反映して移動する場合の処理
	/// </summary>
	/// <param name="info"></param>
	void JudgmentResult(const CollisionMapInfo& info);

	/// <summary>
	/// 天井に接触している場合の処理
	/// </summary>
	/// <param name="info"></param>
	void HitCeilingCollision(CollisionMapInfo& info);

	/// <summary>
	/// 壁に接触している場合の処理
	/// </summary>
	/// <param name="info"></param>
	void HitWallCollision(CollisionMapInfo& info);

	/// <summary>
	/// 接地状態の切り替え処理
	/// </summary>
	/// <param name="info"></param>
	void SwitchingTheGrounding(CollisionMapInfo& info);

	void HandleMapCollisionUp(CollisionMapInfo& info);

	void HandleMapCollisionDown(CollisionMapInfo& info);

	void HandleMapCollisionLeft(CollisionMapInfo& info);

	void HandleMapCollisionRight(CollisionMapInfo& info);

	// 壁滑り・壁ジャンプ
	void UpdateWallSlide(const CollisionMapInfo& info);
	void HandleWallJump(const CollisionMapInfo& info);

	public:
	KamataEngine::WorldTransform& GetWorldTransform() { return worldTransform_; }
	KamataEngine::Vector3 GetPosition() const { return worldTransform_.translation_; }

private:
	// ワールド変換データ
	WorldTransform worldTransform_;

	Model* model_ = nullptr;

	Camera* camera_ = nullptr;

	// マップチップフィールド
	MapChipField* mapChipField_ = nullptr;

	uint32_t textureHandle_ = 0u;

	static inline const float kAcceleration = 0.1f;

	static inline const float kAttenuation = 0.05f;

	static inline const float kLimitRunSpeed = 1.0f;

	// 旋回開始の角度
	float turnFirstRotationY_ = 0.0f;

	// 旋回timer
	float turnTimer_ = 0.0f;

	// 旋回時間
	static inline const float kTimeTurn = 0.3f;

	// 接地状態フラグ
	bool onGround_ = true;

	// 重力加速度(下方向)
	static inline const float kGravityAcceleration = 0.1f;

	// 最大落下速度(下方向)
	static inline const float kLimitFallSpeed = 12.0f;

	// 最大落下速度(上方向)
	static inline const float kJumpAcceleration = 1.5f;

	// キャラクターの当たり判定サイズ
	static inline const float kWidth = 0.8f * 2.0f;
	static inline const float kHeight = 0.8f * 2.0f;

	static inline const float kBlank = 0.1f * 2.0f;

	// 着地時の速度減衰率
	static inline const float kAttenuationLanding = 0.1f;

	// 着地時の速度減衰率
	static inline const float kAttenuationWall = 0.1f;

	// --- 壁けり関連 ---
	bool isWallSliding_ = false;
	float wallJumpCooldown_ = 0.0f; // 同一入力で連続発動しないためのクールダウン
	static inline const float kWallJumpHorizontalSpeed = 1.2f; // 壁から離れるX速度
	static inline const float kWallJumpVerticalSpeed = 2.5f;   // 壁けり時のY速度
	static inline const float kWallSlideMaxFallSpeed = 3.0f;   // 壁滑り中の最大落下速度
	static inline const float kWallJumpCooldownTime = 0.2f;    // クールダウン時間(秒)
};