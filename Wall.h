#pragma once
#include "KamataEngine.h"
#include "MathUtl.h"
// 血管(壁)
class Wall {
public:
	Wall();
	~Wall();
	void Initialize(KamataEngine::Camera* camera, const KamataEngine::Vector3& pos);
	void Update();
	void Draw();

	void UpdateAABB();

	// AABB の取得
	const AABB& GetAABB() const;

	// 位置の取得
	const KamataEngine::Vector3& GetPosition() const;

	// 回転を設定（z軸回転など）
	void SetRotation(const KamataEngine::Vector3& rot);

	// ダメージ蓄積（敵に触れたフレームをカウントし、一定フレームでHPを減らす）
	// 戻り値: true の場合、HP が 0 以下となり破壊される
	bool AccumulateContactFrame();

	int GetHP() const { return hp_; }
	void ResetContactFrames() { contactFrames_ = 0; }

private:
	AABB aabb_{};

	// 体力
	int hp_ = 3;
	// 敵と接触しているフレーム数
	int contactFrames_ = 0;
	static inline const int kRequiredContactFrames_ = 60; // 60フレームでダメージ

private:
	KamataEngine::Model* model_ = nullptr;

	KamataEngine::WorldTransform worldTransform_;

	KamataEngine::Camera* camera_ = nullptr;
};
