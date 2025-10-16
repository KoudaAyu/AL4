#include "MapChipField.h"
#include <fstream>
#include <map>
#include <sstream>

using namespace KamataEngine;

namespace MapChipFieldData {
std::map<std::string, MapChipType> mapChipTable = {
    {"0", MapChipType::kBlank      },
    {"1", MapChipType::kWall       },
    {"2", MapChipType::kPlayerSpawn},
    {"3", MapChipType::kAppleSpawn },
    {"4", MapChipType::kBombSpawn  },
};
}

void MapChipField::Initialize() {}

void MapChipField::ResetMapChipData() {
	mapChipData_.data.clear();
	mapChipData_.data.resize(kNumBlockVirtical);
	for (std::vector<MapChipType>& row : mapChipData_.data) {
		row.resize(kNumBlockHorizontal);
	}
}

void MapChipField::LoadmapChipCsv(const std::string& filePath) {
	ResetMapChipData();

	// ファイル開く
	//   ファイルを開く
	std::ifstream file;
	file.open(filePath);
#ifdef _DEBUG

	assert(file.is_open());

#endif // !_DEBUG

	// マップチップCSV
	std::stringstream mapChipCsv;

	mapChipCsv << file.rdbuf();

	file.close();

	// CSVを読み込む
	for (uint32_t i = 0; i < kNumBlockVirtical; ++i) {
		std::string line;
		getline(mapChipCsv, line);

		std::istringstream line_stream(line);

		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {
			std::string word;
			getline(line_stream, word, ',');

			if (MapChipFieldData::mapChipTable.contains(word)) {
				mapChipData_.data[i][j] = MapChipFieldData::mapChipTable[word];
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

Vector3 MapChipField::GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex) {
	float halfW = kBlockWidth * 0.5f;
	float halfH = kBlockHeight * 0.5f;
	return Vector3(kBlockWidth * xIndex + halfW, kBlockHeight * (kNumBlockVirtical - 1 - yIndex) + halfH, 0);
}