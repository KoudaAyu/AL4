#include "MapChipField.h"

#include <fstream>
#include <map>
#include <sstream>
#include <cmath>

using namespace KamataEngine;

namespace {
std::map<std::string, MapChipType> mapChipTable = {
    {"0", MapChipType::kBlank},
    {"1", MapChipType::kBlock},
    {"2", MapChipType::kReserved2},
    {"3", MapChipType::kEnemySpawn},
    {"4", MapChipType::kEnemySpawnShield},
    {"5", MapChipType::kSpike},
    {"6", MapChipType::kGoal}, 
    {"7", MapChipType::kKey},
    {"8", MapChipType::kIce}, // Ice block
};
}

void MapChipField::Initialize() {}
void MapChipField::Update() {}
void MapChipField::Draw() {}

void MapChipField::ResetMapChipData() {
	mapChipData_.data.clear();
	mapChipData_.data.resize(numBlockVertical_);
	for (std::vector<MapChipType>& mapChipDataRow : mapChipData_.data) {
		mapChipDataRow.resize(numBlockHorizontal_);
	}
}

void MapChipField::LoadMapChipCsv(const std::string& filename) {
	// CSVファイルを開く
	std::ifstream file(filename);

	#ifdef _DEBUG
	assert(file.is_open());
	#endif // _DEBUG

	// ファイル内容を文字列に読み込む
	std::stringstream mapChipCsv;
	mapChipCsv << file.rdbuf();
	file.close();

	// 行ごとに分割してサイズを決定
	std::vector<std::string> lines;
	std::string line;
	while (std::getline(mapChipCsv, line)) {
		// 空行は無視
		if (line.empty()) continue;
		lines.push_back(line);
	}

	if (lines.empty()) {
		// CSV にデータがなければ初期化のみ
		numBlockHorizontal_ = 1;
		numBlockVertical_ = 1;
		ResetMapChipData();
		return;
	}

	// 列数は最大のカンマ区切りトークン数に合わせる
	uint32_t maxCols = 0;
	for (const auto& ln : lines) {
		uint32_t cols = 0;
		std::stringstream ls(ln);
		std::string word;
		while (std::getline(ls, word, ',')) {
			++cols;
		}
		if (cols > maxCols) maxCols = cols;
	}

	// インスタンスのブロック数を CSV に合わせて設定
	SetNumBlockVertical(static_cast<uint32_t>(lines.size()));
	SetNumBlockHorizontal(maxCols);

	// データ領域をリサイズ
	ResetMapChipData();

	// データを埋める（CSV は上から下へ）
	for (uint32_t i = 0; i < numBlockVertical_; ++i) {
		std::stringstream line_stream(lines[i]);
		for (uint32_t j = 0; j < numBlockHorizontal_; ++j) {
			std::string word;
			if (!std::getline(line_stream, word, ',')) {
				// 足りない要素はデフォルトのまま
				break;
			}

			if (mapChipTable.contains(word)) {
				mapChipData_.data[i][j] = mapChipTable[word];
			} else {
				// Unknown value: leave as blank
			}
		}
	}
}

MapChipType MapChipField::GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex) {

	if (xIndex >= numBlockHorizontal_) {
		return MapChipType::kBlank;
	}
	if (yIndex >= numBlockVertical_) {
		return MapChipType::kBlank;
	}

	return mapChipData_.data[yIndex][xIndex];
}

Vector3 MapChipField::GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex) { return Vector3(kBlockWidth * xIndex, kBlockHeight * (numBlockVertical_ - 1 - yIndex), 0); }

IndexSet MapChipField::GetMapChipIndexSetByPosition(const KamataEngine::Vector3& position) {
	IndexSet indexSet = {};

	indexSet.xIndex = static_cast<uint32_t>(std::floor((position.x + kBlockWidth * 0.5f) / kBlockWidth));

	// 画像のロジックに従って Y インデックスを計算
	float preFlipYIndexFloat = (position.y + kBlockHeight * 0.5f) / kBlockHeight;
	uint32_t preFlipYIndex = static_cast<uint32_t>(std::floor(preFlipYIndexFloat));
	indexSet.yIndex = numBlockVertical_ - 1 - preFlipYIndex;

	return indexSet;
}

Rects MapChipField::GetRectByIndex(uint32_t xIndex, uint32_t yIndex) {

	Vector3 center = GetMapChipPositionByIndex(xIndex, yIndex);

	Rects rect = {};

	rect.left = center.x - kBlockWidth * 0.5f;
	rect.right = center.x + kBlockWidth * 0.5f;
	// top は -
	rect.top = center.y + kBlockHeight * 0.5f;
	rect.bottom = center.y - kBlockHeight * 0.5f;

	return rect;
};

Rect MapChipField::GetMovableArea() const {
	// ワールド座標での左端、右端、上端、下端を計算する
	// マップ左下のタイル中心は (0,0) としているため、左端は -0.5 * width
	float left = -kBlockWidth * 0.5f; // 左端の中心座標
	float right = left + static_cast<float>(numBlockHorizontal_) * kBlockWidth - kBlockWidth; // 右端の中心座標

	// Y は上が正で、MapChip の実装では上の行(yIndex==0)が一番上になるように反転している
	// GetMapChipPositionByIndex の計算に合わせる
	float top = (numBlockVertical_ - 1) * kBlockHeight + kBlockHeight * 0.5f;
	float bottom = -kBlockHeight * 0.5f;

	Rect r;
	r.left = left;
	r.right = right;
	r.top = top;
	r.bottom = bottom;
	return r;
}
