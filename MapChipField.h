#pragma once

#include"CameraController.h"
#include "KamataEngine.h"

enum class MapChipType {
	kBlank, // 0
	kBlock, // 1
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

public:
	uint32_t GetNumBlockHorizontal() const { return kNumBlockHorizontal; }
	uint32_t GetNumBlockVertical() const { return kNumBlockVirtical; }

	void SetNumBlockHorizontal(uint32_t count) {
		if (count == 0) {
			count = 1;
		}
		kNumBlockHorizontal = count;
	}

	void SetNumBlockVertical(uint32_t count) {
		if (count == 0) {
			count = 1;
		}
		kNumBlockVirtical = count;
	}

private:
	// ブロックのサイズ
	static inline const float kBlockWidth = 2.0f;
	static inline const float kBlockHeight = 2.0f;

	static inline uint32_t kNumBlockHorizontal = 20;
	static inline uint32_t kNumBlockVirtical = 10;

	MapChipData mapChipData_;
};