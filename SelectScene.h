#pragma once

#include "KamataEngine.h"
#include <vector>

class CameraController; // forward
class Skydome; // forward

class SelectScene {
public:
    SelectScene() = default;
    ~SelectScene();

    void Initialize();
    void Update();
    void Draw();

    bool IsFinished() const { return finished_; }

    // IsFinished() が true の後に選択されたステージインデックスを返す（未選択時は -1）
    int GetChosenStageIndex() const { return chosenStage_; }

    // Prevent the internal player from recognizing a jump on the next update (used after scene switches)
    void SuppressPlayerNextJump();

private:
    bool finished_ = false;

    // セレクトシーンでプレイヤー描画に使うカメラ
    KamataEngine::Camera camera_;

    // 表示・操作するプレイヤーインスタンス
    class Player* player_ = nullptr;

    // プレイヤーがタイルと相互作用するためのマップチップフィールド（任意）
    class MapChipField* mapChipField_ = nullptr;

    // ブロック描画に使うモデル
    KamataEngine::Model* blockModel_ = nullptr;
    // ステージノード描画に使うモデル
    KamataEngine::Model* stageModel_ = nullptr;
    // Special model for Stage1 (use Stage1.obj)
    KamataEngine::Model* stage1Model_ = nullptr;
    // Special models for Stage2 and Stage3
    KamataEngine::Model* stage2Model_ = nullptr;
    KamataEngine::Model* stage3Model_ = nullptr;

    // スカイドーム
    Skydome* skydome_ = nullptr;

    // ブロックのワールド変換（行×列）
    std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;

    // GameSceneと同様にプレイヤーを追従するカメラコントローラ
    CameraController* cameraController_ = nullptr;

    // ステージ選択用データ
    std::vector<KamataEngine::Vector3> stagePositions_; // kStageチップの位置（左→右でソート済）
    std::vector<KamataEngine::WorldTransform*> stageWorldTransforms_; // ステージノードの描画用ワールド変換（オプション）
    int highlightedStage_ = -1; // 現在ハイライト中のステージインデックス（プレイヤーが近い）
    int chosenStage_ = -1; // プレイヤーが決定したステージインデックス
    float stageDetectRadius_ = 1.0f; // プレイヤーがステージノードに『到達』したとみなす距離閾値

    // 二重トリガを防ぐ簡易デバウンス
    float inputCooldown_ = 0.18f; // 秒
    float inputTimer_ = 0.0f;

    // 選択演出用の状態
    bool transitioning_ = false; // 選択演出中かどうか
    float transitionTimer_ = 0.0f; // 残り時間
    float transitionDuration_ = 1.2f; // 演出の合計時間
    float transitionProgress_ = 0.0f; // 0..1
    KamataEngine::Vector3 transitionCameraStart_ = {0.0f, 0.0f, -50.0f};
    KamataEngine::Vector3 transitionCameraTarget_ = {0.0f, 0.0f, -18.0f};

    // フェードコントローラ
    class Fade* fade_ = nullptr;
    bool fadeStarted_ = false; // start fade only after transition animation ends

    // Audio handles for Select scene BGM
    uint32_t bgmDataHandle_ = 0u;
    uint32_t bgmVoiceHandle_ = 0u;
    bool bgmStarted_ = false;

    // ステージ番号PNGの描画用
    std::vector<uint32_t> stageNumberTexHandles_; // TextureManager::Load で読み込んだハンドル
    std::vector<KamataEngine::Sprite*> stageNumberSprites_; // 画面左から 1,2,3... と並べて描画

    // 左下UI用スプライト（Resources/Sprite/SelectScene/LT.png を想定）
    uint32_t ltTexHandle_ = 0u;
    KamataEngine::Sprite* ltSprite_ = nullptr;

    // キーボード用Qアイコン
    uint32_t qTexHandle_ = 0u;
    KamataEngine::Sprite* qSprite_ = nullptr;

    // 最後に使われた入力デバイスを記録（フレーム間で持続させる）
    enum class InputMode { kUnknown = 0, kGamepad, kKeyboard };
    InputMode lastInputMode_ = InputMode::kUnknown;

    uint32_t textureHandleStage1_ = 0u; 
    uint32_t textureHandleStage2_ = 0u;
    uint32_t textureHandleStage3_ = 0u;
};
