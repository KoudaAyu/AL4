#pragma once
#include "KamataEngine.h"
#include<vector>

enum class MapChipType {
	kBlank,
	kWall,
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

private:
	static inline const float kBlockWidth = 2.0f;
	static inline const float kBlockHeight = 2.0f;

	static inline const uint32_t kNumBlockVirtical = 10;
	static inline const uint32_t kNumBlockHorizontal = 20;

	MapChipData mapChipData_;
};


