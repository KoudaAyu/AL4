#pragma once
#include "KamataEngine.h"
#include<vector>

enum class MapChipType {
	kBlank,
	kWall,
	kPlayerSpawn, 
	kAppleSpawn,
	kBombSpawn,
};

struct MapChipData {
	std::vector<std::vector<MapChipType>> data;
};


class MapChipField {
public:
	
	void Initialize();
	//リセット
	void ResetMapChipData();
	//読み込み
	void LoadmapChipCsv(const std::string& fielPath);
	//種類別取得
	MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex);
	//マップチップ座標の取得
	KamataEngine::Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex);

	const uint32_t GetNumBlockVirtical() const { return kNumBlockVirtical; }
	const uint32_t GetNumBlockHorizontal() const { return kNumBlockHorizontal; }

	const float GetBlockWidth() const { return kBlockWidth; }
	const float GetBlockHeight() const { return kBlockHeight; }

	 bool IsMovable(int gridX, int gridY) const {
		// 例: フィールド配列が0なら移動可
		// field_は2次元配列（vector<vector<int>>等）で定義されていると仮定
		if (gridX < 0 || gridY < 0 || gridX >= fieldWidth_ || gridY >= fieldHeight_)
			return false;
		return field_[gridY][gridX] == 0;
	}

private:
	static inline const float kBlockWidth = 2.0f;
	static inline const float kBlockHeight = 2.0f;

	static inline const uint32_t kNumBlockVirtical = 10;
	static inline const uint32_t kNumBlockHorizontal = 20;

	MapChipData mapChipData_;

	int fieldWidth_ = 0;
	int fieldHeight_ = 0;
	std::vector<std::vector<int>> field_; // 0:通行可, 1:壁
};


