# Research: Sprout Engine（spec v2）

各項目: Decision / Rationale / Alternatives considered

## R1. 基盤: SFML 3.0
- **Decision**: SFML 3.0（graphics/window/audio/system）
- **Rationale**: C++17 以降前提の新 API、イベントの型安全化、miniaudio ベースの音声で空間化対応。利用者指定
- **Alternatives**: SFML 2.6（旧 API）、SDL3（利用者指定外）

## R2. 物理: Box2D v3
- **Decision**: Box2D 3.x を `sprout_physics` で完全にラップ
- **Rationale**: 2D 物理の事実上の標準。v3 は C API でハンドル設計（エンジンのハンドル方針と一致）、連続衝突・ジョイント・センサー・レイキャスト・決定性（同一バイナリ・同一入力）を満たす
- **Alternatives**: 自作 AABB 物理（FR-350〜358 を満たせない）、Chipmunk2D（保守頻度が低い）

## R3. ゲームコードの実行方式: ゲームモジュール DLL + シャドウコピー
- **Decision**: 学生コードを `<Game>.dll` としてビルド。エディタは `Library/ModuleCache/<hash>/` へコピーして `LoadLibrary`。変更検知（efsw）→ 現ワールドを `WorldSnapshot` 化 → 全 Behavior 破棄 → `FreeLibrary` → 新 DLL ロード → 型再登録 → 復元（フィールド名一致で値移行）
- **Rationale**: エディタを 1 つ配布すればよく、VS で書いてビルドするだけで反映（SC-004）。PDB ロックはシャドウコピー + `/PDBALTPATH` で回避
- **Alternatives**: 学生プロジェクトごとにエディタをビルド（ビルド時間が長く SC-004 未達）、Live++ 等（商用）、スクリプト言語（スコープ外）

## R4. ABI 方針
- **Decision**: エンジン DLL は C++ API を `SPROUT_API` で輸出。エンジン・エディタ・ゲームは同一 MSVC 系列（v143/v145 は互換）+ 同一構成（Debug/Release）でビルドすることを `SproutGameModule.cmake` が検査。モジュール入口のみ `extern "C" sproutModuleEntry(const ModuleHostInfo*)`
- **Rationale**: 学生に C API を書かせない。v14x 系の ABI 互換性で VS2022/2026 混在も可能
- **Alternatives**: 全面 C ABI（使いにくい）

## R5. オブジェクト格納: ハンドル + 型別プール
- **Decision**: `ObjectTable`（スロット配列 + フリーリスト + 世代）と型別 `ComponentPool<T>`（密配列 + スロット間接）。ER 図 2 参照
- **Rationale**: 破棄済み参照の検出（FR-104）、キャッシュ効率（SC-005）、ポインタを利用者に渡さない（Facade）
- **Alternatives**: `unique_ptr` の木（ダングリング検出不可）、EnTT（内部依存として可能だが学習教材としての可読性を優先し自作）

## R6. リフレクション
- **Decision**: `SPROUT_BEHAVIOR(T)` と `SPROUT_PROPERTY(member, attrs...)`（メンバ宣言の直前に置くマクロ）で `TypeInfo/FieldInfo` を生成。組込みコンポーネントは `detail` 側で手書き登録
- **Rationale**: 1 行登録。オフセット + 型タグ方式でシリアライズ・インスペクタ・Undo・Prefab 上書きが共通化（FR-802）
- **Alternatives**: `declareProperties()` 仮想関数のみ（v1 案。冗長だが互換用に残す）、外部コード生成（ビルド複雑化）

## R7. シリアライズと版移行
- **Decision**: 開発時は JSON（nlohmann-json）、書き出し時は同一スキーマをバイナリ化して zstd 圧縮した `.spak`。各ファイルに `formatVersion`、`Migrator` を版ごとに連鎖適用
- **Rationale**: 人が読める（FR-553）＋配布時は高速・小容量
- **Alternatives**: YAML（遅い・依存増）、FlatBuffers（スキーマ管理が教材として重い）

## R8. 描画パイプライン
- **Decision**: 毎フレーム描画対象を `RenderQueueItem{sortKey}` に収集 → 基数ソート → 同一テクスチャ/マテリアル連続区間を `sf::VertexArray` 1 回で描画。カメラごとに `sf::View`、ライトは光量マップ（RenderTexture + 加算 + 乗算合成）、ポスト FX は RenderTexture のピンポン
- **Rationale**: SC-005（10k スプライト）を描画呼出し数十回で達成
- **Alternatives**: `sf::Sprite` 個別描画（10k で draw call 10k、未達）

## R9. エディタ UI
- **Decision**: Dear ImGui docking + ImGui-SFML、ノードグラフは imnodes、ファイルダイアログは nativefiledialog-extended、日本語フォント Noto Sans JP（OFL）同梱、テーマ Dark/Light
- **Alternatives**: Qt（重い・ライセンス）

## R10. Undo/Redo
- **Decision**: Command パターン。`SetPropertyCommand` はプロパティパス + 旧/新値（variant）で汎用化、ドラッグは `mergeKey` で統合、複数対象は `CompositeCommand`。履歴 500 手、保存点を記録して dirty 判定
- **Alternatives**: スナップショット差分（大シーンでメモリ大）

## R11. プレハブ
- **Decision**: Unity 方式（インスタンスは Prefab の fileId を参照し、差分を PropertyOverride / Added / Removed で保持）。入れ子・派生（Variant）対応、循環は DAG 検査で拒否
- **Alternatives**: 展開コピーのみ（変更伝播できない）

## R12. 入力
- **Decision**: SFML のキーボード/マウス/ジョイスティック + Windows では XInput 直読み（ボタン配置の正規化）、SDL GameControllerDB 形式の対応表で汎用パッドを正規化。アクションマップ方式
- **Alternatives**: 生入力のみ（FR-450 未達）

## R13. 音声
- **Decision**: SFML Audio（`sf::Sound` プール + `sf::Music` ストリーム）を `VoicePool` で管理、ミキサーグループは乗算音量の木、空間化は SFML の 3D 音源を z=0 で使用
- **Alternatives**: miniaudio 直接（SFML 内部と二重化）

## R14. ファイル監視・非同期
- **Decision**: efsw でアセット変更監視。ジョブシステムは `std::jthread` ベースの固定ワーカー + ワークスティーリング無しの単純キュー（教材性重視）、メインスレッド完了コールバック
- **Alternatives**: enkiTS / Taskflow（高性能だが教材として不透明）

## R15. プロファイラ
- **Decision**: 自前の軽量マーカー（リングバッファ 600 フレーム）をエディタで可視化。`SPROUT_WITH_TRACY=ON` で Tracy にも転送
- **Alternatives**: Tracy のみ（外部ツールが必要で学生に重い）

## R16. ビルド・CI
- **Decision**: CMake 4.2+、プリセット `vs2022`, `vs2026`, `ninja-msvc`, `ci`。GitHub Actions `windows-2022` で build/test/lint/docs。VS2026 は自前ランナー（提供され次第ホスト型へ移行）
- **Alternatives**: MSBuild ソリューション直管理（再現性が低い）

## R17. テスト
- **Decision**: GoogleTest（ロジック）、ImGui Test Engine（エディタ操作）、決定性テストは同一入力列で物理状態ハッシュ比較、往復テストは全アセット型について Save→Load→Save のバイト一致
- **Alternatives**: 手動 QA のみ

## R18. 命名規則の補足（constitution 改訂提案）
- 公開マクロを `SPROUT_BEHAVIOR`, `SPROUT_PROPERTY`, `SPROUT_GAME_MODULE` の 3 つに拡大
- 輸出マクロ `SPROUT_API` は内部用（利用者は書かない）
