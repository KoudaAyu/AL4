#include "MapChipField.h"

#include <fstream>
#include <map>
#include <sstream>

using namespace KamataEngine;

namespace {
std::map<std::string, MapChipType> mapChipTable = {
    {"0", MapChipType::kBlank},
    {"1", MapChipType::kBlock},
};
}

void MapChipField::Initialize() {}
void MapChipField::Update() {}
void MapChipField::Draw() {}

void MapChipField::ResetMapChipData() {
	mapChipData_.data.clear();
	mapChipData_.data.resize(kNumBlockVirtical);
	for (std::vector<MapChipType>& mapChipDataRow : mapChipData_.data) {
		mapChipDataRow.resize(kNumBlockHorizontal);
	}
}

void MapChipField::LoadMapChipCsv(const std::string& filename) {
	ResetMapChipData();

	// CSVファイルを開く
	std::ifstream file;
	file.open(filename);
	assert(file.is_open());

	// マップチップCSV
	std::stringstream mapChipCsv;
	mapChipCsv << file.rdbuf();

	// ファイルを閉じる
	file.close();

	// CSVからマップチップデータを読み込む
	for (uint32_t i = 0; i < kNumBlockVirtical; ++i) {
		std::string line;
		getline(mapChipCsv, line);

		// 1行分の文字列をカンマで区切って読み込む
		std::stringstream line_stream(line);
		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {

			std::string word;
			getline(line_stream, word, ',');

			if (mapChipTable.contains(word)) {
				mapChipData_.data[i][j] = mapChipTable[word];
			}
		}
	}
}

MapChipType MapChipField::GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex) {

	if (xIndex < 0 || kNumBlockHorizontal - 1 < xIndex) {
		return MapChipType::kBlank;
	}
	if (yIndex < 0 || kNumBlockVirtical - 1 < yIndex) {
		return MapChipType::kBlank;
	}

	return mapChipData_.data[yIndex][xIndex];
}

Vector3 MapChipField::GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex) { return Vector3(kBlockWidth * xIndex, kBlockHeight * (kNumBlockVirtical - 1 - yIndex), 0); }

MapChipField::IndexSet MapChipField::GetMapChipIndexByPosition(const KamataEngine::Vector3& position) {
	IndexSet indexSet = {};

	indexSet.xIndex = static_cast<uint32_t>(std::floor((position.x + kBlockWidth * 0.5f) / kBlockWidth));

	// 画像のロジックに従って Y インデックスを計算
	float preFlipYIndexFloat = (position.y + kBlockHeight * 0.5f) / kBlockHeight;
	uint32_t preFlipYIndex = static_cast<uint32_t>(std::floor(preFlipYIndexFloat));
	indexSet.yIndex = kNumBlockVirtical - 1 - preFlipYIndex;

	return indexSet;
}

Rect MapChipField::GetRectByIndex(uint32_t xIndex, uint32_t yIndex) {

	Vector3 center = GetMapChipPositionByIndex(xIndex, yIndex);

	Rect rect = {};

	rect.left = center.x - kBlockWidth * 0.5f;
	rect.right = center.x + kBlockWidth * 0.5f;
	// top は -
	rect.top = center.y + kBlockHeight * 0.5f;
	rect.bottom = center.y - kBlockHeight * 0.5f;

	return rect;
};
