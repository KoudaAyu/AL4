#pragma once
#define NOMINMAX
#include "KamataEngine.h"

#include <algorithm>
#include <assert.h>
#include <cstdint>
#include <numbers>

#include"AABB.h"
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
class Enemy;
class CameraController; // forward declaration

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

	enum class Behavior {
		kUnknown,
		kRoot,   // 通常
		kAttack, // 攻撃
	};

	// コンストラクタ
	Player();

	// デストラクタ
	~Player();

	// 初期化
	void Initialize(Camera* camera, const Vector3& position);

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

	void OnCollision(Enemy* enemy);

	void UpdateAABB();

	/// <summary>
	/// 緊急回避
	/// </summary>
	void EmergencyAvoidance();

	//攻撃

	// 行動初期化
	void BehaviorRootInitialize();

	// 攻撃行動初期化
	void BehaviorAttackInitialize();

	//通常行動更新
	void BehaviorRootUpdate();

	//攻撃行動更新
	void BehaviorAttackUpdate();


	LRDirection GetLRDirection() const { return lrDirection_; }

	// Attack accessors
	bool IsAttacking() const { return behavior_ == Behavior::kAttack; }
	AABB GetAttackAABB() const;

public:
	KamataEngine::WorldTransform& GetWorldTransform() { return worldTransform_; }
	KamataEngine::Vector3 GetPosition() const { return worldTransform_.translation_; }

	AABB& GetAABB() { return aabb_; }

	bool isAlive() const { return isAlive_; }

	// 死亡一時停止中かどうか
	bool IsDying() const { return isDying_; }

	// カメラコントローラの参照を渡す（カメラシェイクを呼ぶため）
	void SetCameraController(CameraController* controller) { cameraController_ = controller; }

	// HP accessor
	int GetHP() const { return hp_; }

private:
	// ワールド変換データ
	WorldTransform worldTransform_;

	Model* model_ = nullptr;

	Model* attackModel_ = nullptr;

	// Player が自身で生成した Model を所有しているか
	bool ownsModel_ = false;

	Camera* camera_ = nullptr;

	// マップチップフィールド
	MapChipField* mapChipField_ = nullptr;

	AABB aabb_;

	// 現在の行動状態
	Behavior behavior_ = Behavior::kRoot;
	Behavior behaviorRequest_ = Behavior::kUnknown;
	
private:

	uint32_t textureHandle_ = 0u;

	//加速量
	static inline const float kAcceleration = 0.05f;

	// 速度減衰（慣性制御）
	// 値を増やして慣性を減らす（入力停止時にすばやく速度を落とす）
	static inline const float kAttenuation = 0.25f; 

	//横移動の最大速度
	static inline const float kLimitRunSpeed = 0.5f;

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


	// ジャンプ用の上向き速度（地上からの一段目）
	// 固定値で上向き速度を設定することで、二段ジャンプの高さが入力時の落下速度に依存しないようにする
	static inline const float kJumpVelocityGround = 1.0f; // 調整可能: 一段目の上向き速度
	// 空中での二段ジャンプ時の上向き速度（地上ジャンプよりやや小さめに）
	static inline const float kJumpVelocityAir = 0.8f; // 調整可能: 二段目の上向き速度

	// 最大落下速度(上方向)
	static inline const float kJumpAcceleration = 1.0f;


	// キャラクターの当たり判定サイズ
	static inline const float kWidth = 0.8f * 2.0f;
	static inline const float kHeight = 0.8f * 2.0f;

	static inline const float kBlank = 0.1f * 2.0f;

	// 着地時の速度減衰率
	static inline const float kAttenuationLanding = 0.35f; 

	// 着地時の速度減衰率
	static inline const float kAttenuationWall = 0.2f;

	// --- 壁けり関連 ---
	bool isWallSliding_ = false;
	float wallJumpCooldown_ = 0.0f; // 同一入力で連続発動しないためのクールダウン

	static inline const float kWallJumpHorizontalSpeed = 0.6f; // 壁から離れるX速度
	static inline const float kWallJumpVerticalSpeed = 1.4f;   // 壁けり時のY速度 (reduced)

	static inline const float kWallJumpHorizontalSpeed2 = 0.3f; // second jump horizontal
	static inline const float kWallJumpVerticalSpeed2 = 0.9f; // second jump vertical

	static inline const float kWallSlideMaxFallSpeed = 3.0f;   // 壁滑り中の最大落下速度
	static inline const float kWallJumpCooldownTime = 0.1f;    // クールダウン時間(秒) (reduced)
	static inline const float kWallJumpHorizontalDamp = 0.6f; 

	int wallJumpCount_ = 0;
	static inline const int kMaxWallJumps = 2;

	WallSide lastWallSide_ = WallSide::kNone;

	static inline const float kWallContactGraceTime = 0.1f; 
	float wallContactGraceTimer_ = 0.0f;

	bool isAlive_ = true; 	

	//攻撃ギミックの経過時間
	uint32_t attackParameter_ = 0;

	float static inline const kAttackDuration = 10; // 攻撃動作の継続時間(フレーム)

	// attack hitbox
	static inline const float kAttackReach = 1.0f; // 前方への到達距離
	volatile static inline const float kAttackWidth = 1.2f; // 当たり判定の幅(横)
	static inline const float kAttackHeight = 0.8f; // 当たり判定の高さ

	// 攻撃時の短距離ダッシュ設定
	static inline const float kAttackDashSpeed = 1.5f; // 攻撃開始時のダッシュ速度
	static inline const int kAttackDashFrames = 6;     // ダッシュが続くフレーム数

	// カメラコントローラ参照（シェイク呼び出し用）
	CameraController* cameraController_ = nullptr;

	XINPUT_STATE state;

    // 前フレームで右トリガーが押されていたか（単発入力判定用）
    bool prevRightTriggerPressed_ = false;


	// 二段ジャンプ関連
	static inline const int kMaxJumps = 2; // 最大ジャンプ回数（地上から含む）
	int jumpCount_ = 0; // 現在のジャンプ回数
	// ゲームパッドのAボタンの前フレーム状態（ライズエッジ検出用）
	bool prevAButtonPressed_ = false;
	// キーボードジャンプキーの前フレーム状態（ライズエッジ検出用）
	bool prevJumpKeyPressed_ = false;

	// --- Emergency dodge (Eキー) ---
	bool isDodging_ = false;
	float dodgeTimer_ = 0.0f;
	float dodgeCooldown_ = 0.0f;
	static inline const float kDodgeDuration = 0.15f; // seconds
	static inline const float kDodgeSpeed = 2.0f; // dash speed
	static inline const float kDodgeCooldownTime = 0.5f; // seconds

	// 死亡遷移用フラグ（接触直後に一瞬静止してから死亡扱いにする）
	bool isDying_ = false;
	float deathDelayTimer_ = 0.0f;
	static inline const float kDeathDelay = 0.5f; // seconds: 一瞬静止する時間

	// --- HP ---
	static inline const int kMaxHP = 3;
	int hp_ = kMaxHP;

	bool invincible_ = false;
	float invincibleTimer_ = 0.0f;
	static inline const float kInvincibleDuration = 1.0f;

};