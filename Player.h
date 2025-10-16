#pragma once
#define NOMINMAX
#include "AABB.h"
#include "KamataEngine.h"
#include <Windows.h>
#include <deque>

enum class LRDirection { Left, Right, Unknown };
enum class UDDirection { Up, Down, Unknown };
class MapChipField;
class Player {
public:
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3 position);
	void Update();
	void Draw();

	void Grow();           // 体を伸ばす
	void RemoveLastPart(); // 体を1つ減らす

	void UpdateAABB();

	// 爆弾関係
	void EatBomb();         // 爆弾を食べた時の処理
	void UpdateBomb();      // 爆弾進行の更新
	void DetachBombParts(); // 切り離し処理


	const KamataEngine::Vector3& GetPosition() const { return worldTransform_.translation_; }
	const std::vector<KamataEngine::Vector3>& GetBodyParts() const { return bodyParts_; }
	const std::deque<KamataEngine::WorldTransform>& GetWallTransforms() const { return wallTransforms_; }

	bool IsBombActive() const { return bombActive_; } // 爆弾状態取得

	const AABB& GetAABB() const { return playerAABB; }

	void SetAlive(bool alive) { isAlive_ = alive; }
	bool IsAlive() const { return isAlive_; }

private:
	KamataEngine::Vector2 gridPos_ = {0.0f, 0.0f};
	KamataEngine::Vector2 targetGridPos_ = {0.0f, 0.0f};

	KamataEngine::Vector2 direction_ = {1.0f, 0.0f}; // 右向きで開始
	KamataEngine::Vector2 nextDirection_ = {1.0f, 0.0f};

	bool isMoving_ = false;
	float moveTimer_ = 0.0f;

	MapChipField* mapChipField_ = nullptr;

	KamataEngine::Vector3 startPos_;
	KamataEngine::Vector3 endPos_;

	static constexpr float kMoveDuration = 0.5f;

private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;
	uint32_t textureHandle_ = 0u;

	KamataEngine::Vector3 velocity_ = {};

	bool isAlive_ = true;

	// 向き
	LRDirection lrDirection_ = LRDirection::Unknown;
	UDDirection udDirection_ = UDDirection::Unknown;

	bool lrKnown_ = false;
	bool udKnown_ = false;

	// 慣性移動
	static inline const float kAcceleration = 0.05f;
	// 速度減少率
	static inline const float kAttenuation = 0.1f;

	// 速度制限
	static inline const float kLimitSpeed = 0.05f;

	// 旋回開始時の角度
	float turnFirstRotationY_ = 0.0f;

	// 旋回タイマー
	float turnTimer_ = 0.0f;

	// 旋回にかかる時間<秒>
	static inline const float kTimeTurn = 0.3f;

	// 体の増加部分
	std::vector<KamataEngine::Vector3> bodyParts_;
	std::deque<KamataEngine::WorldTransform> bodyPartTransforms_;

	// Playerの切り捨てた部分を壁にする
	std::vector<KamataEngine::Vector3> wallPositions_;
	std::deque<KamataEngine::WorldTransform> wallTransforms_; // ←追加

	// 追従遅延フレーム数
	static constexpr size_t kFollowDelay = 30; // 例: 5フレーム遅れ

	// 頭の座標履歴
	std::deque<KamataEngine::Vector3> headHistory_;

	//まずのサイズと一致させる
	static constexpr float unitLength = 2.0f;

	AABB playerAABB;

	// 爆弾関連
	bool bombActive_ = false;                    // 爆弾を飲み込んでいるか
	int bombProgress_ = 0;                       // 爆弾進行度（何個分赤くなったか）
	int bombStartIndex_ = -1;                    // 爆弾が始まる体のインデックス
	float bombTimer_ = 0.0f;                     // 爆弾進行用タイマー
	static constexpr float kBombStepTime = 0.5f; // 1マス赤くなるまでの時間

	float deltaTime_ = 1.0f / 60.0f;

	KamataEngine::ObjectColor redColor;

	KamataEngine::ObjectColor normalColor;

	bool justGrew_ = false;
};
