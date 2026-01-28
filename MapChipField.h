#pragma once

#include"CameraController.h"
#include "KamataEngine.h"

enum class MapChipType {
	kBlank = 0, // 0
	kBlock = 1, // 1
	kReserved2 = 2, // 2: reserved / kept open
	kEnemySpawn = 3,       // 3: 敵 スポーン場所 (右向き)
	kEnemySpawnShield = 4, // 4: シールド持ち敵 スポーン場所
	kSpike = 5, //5: 棘
	kGoal = 6,  //6: ゴール
	kKey = 7,   //7: 鍵
	kIce = 8,   //8: 氷ブロック（Blockと同じ挙動、モデルのみIce）
	kLadder = 9, //9: はしご（追加）
	kStage = 10, //10: ステージノード（SelectScene でステージ選択用）
	kShooter = 11, //11: 弾を発射する敵（Spikeのようにマップに置く）
	kEnemySpawnLeft = 12, //12: 敵 スポーン場所 (左向き)
	kEnemySpawnShieldRight = 13, //13: シールド持ち敵 スポーン場所 (右向き)
	kShooterRight = 14, //14: 右向きの Shooter 敵 スポーン場所
};

struct MapChipData {
	std::vector<std::vector<MapChipType>> data;
};

struct IndexSet {
	uint32_t xIndex;
	uint32_t yIndex;
};

struct Rects {
	float left;   // 左端
	float right;  // 右端
	float top;    // 下端
	float bottom; // 上端
};

class MapChipField {

public:
	

public:

	void Initialize();
	void Update();
	void Draw();

	/// <summary>
	/// マップチップのデータをリセットする
	/// </summary>
	void ResetMapChipData();

	/// <summary>
	/// CSVファイルからマップチップデータを読み込む
	/// </summary>
	/// <param name="filename">CSVの名前</param>
	void LoadMapChipCsv(const std::string& filename);

	/// <summary>
	/// マップチップ種別の取得
	/// </summary>
	/// <param name="xIndex">横</param>
	/// <param name="yIndex">縦</param>
	/// <returns>マップチップに対応したモデルの描画が可能になる</returns>
	MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex);

	/// <summary>
	/// マップチップ座標の取得
	/// </summary>
	/// <param name="Index">横</param>
	/// <param name="yIndex">縦</param>
	/// <returns>マップチップのワールド座標を取得する関数</returns>
	KamataEngine::Vector3 GetMapChipPositionByIndex(uint32_t Index, uint32_t yIndex);

	/// <summary>
	/// 座標からマップチップ番号を計算
	/// </summary>
	/// <param name="position">座標指定</param>
	/// <returns></returns>
	IndexSet GetMapChipIndexSetByPosition(const KamataEngine::Vector3& position);

	Rects GetRectByIndex(uint32_t xIndex, uint32_t yIndex);

	/// <summary>
	/// CSV のサイズに基づいてカメラの移動可能領域を返す
	/// </summary>
	/// <returns>CameraController::Rect と同等の構造体 (left,right,top,bottom)</returns>
	Rect GetMovableArea() const;

	/// <summary>
	/// 指定インデックスに対応する摩擦係数を取得する
	/// 返り値は 0.0f (滑りやすい/氷) から 1.0f (摩擦が高い/普通のブロック) を想定
	/// </summary>
	float GetFrictionCoefficientByIndex(uint32_t xIndex, uint32_t yIndex);

	/// <summary>
	/// 指定ワールド座標の下にあるマップチップの摩擦係数を取得する
	/// </summary>
	float GetFrictionCoefficientByPosition(const KamataEngine::Vector3& position);

public:
	uint32_t GetNumBlockHorizontal() const { return numBlockHorizontal_; }
	uint32_t GetNumBlockVertical() const { return numBlockVertical_; }

	void SetNumBlockHorizontal(uint32_t count) {
		if (count == 0) {
			count = 1;
		}
		numBlockHorizontal_ = count;
	}

	void SetNumBlockVertical(uint32_t count) {
		if (count == 0) {
			count = 1;
		}
		numBlockVertical_ = count;
	}

private:
	// ブロックのサイズ
	static inline const float kBlockWidth = 2.0f;
	static inline const float kBlockHeight = 2.0f;

	// ブロック数はインスタンスメンバにして CSV に合わせて変更可能にする
	uint32_t numBlockHorizontal_ = 20;
	uint32_t numBlockVertical_ = 10;

	MapChipData mapChipData_;
};