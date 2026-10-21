> ⚠️ **OUTDATED**: 本 tasks は spec v1 に基づく。plan 再生成後に `/speckit-tasks` で再生成すること。

# Tasks: 学生向け 2D ゲームエンジン + エディタ（Sprout Engine）

**Input**: `specs/001-student-2d-engine/` の plan.md, spec.md, research.md, data-model.md, contracts/
**Tests**: 必須（FR-019 / constitution IV）。各ストーリーでテストを先に書き、失敗を確認してから実装する。
**Format**: `- [ ] Txxx [P] [USn] 説明 (パス)` — [P] は並行実行可

## Phase 1: Setup

- [ ] T001 ルート CMake とターゲット `sprout`(静的) / `SproutEditor` / `sprout_tests` を作成 (CMakeLists.txt, engine/CMakeLists.txt, editor/CMakeLists.txt, tests/CMakeLists.txt)
- [ ] T002 [P] VS2022 / VS2026 / ninja-msvc の configure・build・test プリセットを作成 (CMakePresets.json)
- [ ] T003 [P] vcpkg マニフェストに sfml, imgui[docking-experimental], imgui-sfml, nlohmann-json, gtest を記載 (vcpkg.json)
- [ ] T004 [P] constitution II の命名規則を readability-identifier-naming で設定 (.clang-tidy)
- [ ] T005 [P] 書式設定と `sprout_lint` ターゲットを追加 (.clang-format, cmake/SproutLint.cmake)
- [ ] T006 [P] `/W4 /permissive- /WX /utf-8` を共通警告設定に追加 (cmake/SproutWarnings.cmake)

## Phase 2: Foundational（全ストーリーの前提）

- [ ] T007 [P] Vector2 / Color / Rect のテストを書く (tests/unit/math/Vector2Tests.cpp, tests/unit/math/RectTests.cpp)
- [ ] T008 [P] Vector2, Color, Rect を実装 (engine/include/sprout/math/Vector2.hpp, Color.hpp, Rect.hpp)
- [ ] T009 [P] Result<T> と Error のテストと実装 (tests/unit/core/ResultTests.cpp, engine/include/sprout/core/Result.hpp)
- [ ] T010 [P] Log の公開 API と LogSink（エディタ購読可能）を実装 (engine/include/sprout/core/Log.hpp, engine/src/core/LogSink.cpp)
- [ ] T011 ObjectHandle と ObjectRegistry（世代番号・遅延破棄）のテスト (tests/unit/scene/ObjectRegistryTests.cpp)
- [ ] T012 ObjectHandle / ObjectRegistry を実装 (engine/src/scene/ObjectHandle.hpp, engine/src/scene/ObjectRegistry.cpp)
- [ ] T013 Component 基底と Transform（親子・ワールド座標）のテスト (tests/unit/component/TransformTests.cpp)
- [ ] T014 Component と Transform を実装 (engine/include/sprout/component/Component.hpp, Transform.hpp, engine/src/component/Transform.cpp)
- [ ] T015 Scene / GameObject のテスト（生成・検索・破棄・無効ハンドル・部品追加削除）(tests/unit/scene/SceneTests.cpp, tests/unit/scene/GameObjectTests.cpp)
- [ ] T016 Scene（PImpl）と GameObject ハンドルを実装 (engine/include/sprout/scene/Scene.hpp, GameObject.hpp, engine/src/scene/SceneImpl.cpp)
- [ ] T017 [P] AssetCache<T> と FallbackAssets のテスト（重複読込なし・失敗時代替）(tests/unit/asset/AssetCacheTests.cpp)
- [ ] T018 [P] AssetCache<T>, TextureLoader, FallbackAssets を実装 (engine/src/asset/AssetCache.hpp, TextureLoader.cpp, FallbackAssets.cpp)

**Checkpoint**: ヘッドレスでシーン・オブジェクト・部品がテストで動く

## Phase 3: User Story 1 - コードだけで最初のゲーム (P1) 🎯 MVP

**Independent Test**: HelloSprout で矢印キー移動、画像欠損で代替表示

- [ ] T019 [P] [US1] InputState のテスト（押下/継続/解放のフレーム遷移）(tests/unit/input/InputStateTests.cpp)
- [ ] T020 [P] [US1] Behavior ライフサイクル呼出順のテスト（onStart→onUpdate→onDestroy、更新中の生成/破棄）(tests/unit/component/BehaviorTests.cpp)
- [ ] T021 [P] [US1] GameLoop の固定刻み・0.25s クランプのテスト (tests/unit/core/GameLoopTests.cpp)
- [ ] T022 [US1] Key / MouseButton / Input と InputState を実装 (engine/include/sprout/input/Input.hpp, Key.hpp, MouseButton.hpp, engine/src/input/InputState.cpp)
- [ ] T023 [US1] Behavior 基底を実装 (engine/include/sprout/component/Behavior.hpp, engine/src/component/Behavior.cpp)
- [ ] T024 [US1] GameLoop と Window（SFML ラップ）を実装 (engine/src/core/GameLoop.cpp, engine/src/core/Window.cpp)
- [ ] T025 [US1] Renderer / Camera と SpriteRenderer・TextRenderer を実装 (engine/src/render/Renderer.cpp, engine/src/render/Camera.cpp, engine/include/sprout/component/SpriteRenderer.hpp, TextRenderer.hpp)
- [ ] T026 [US1] Game / GameConfig / Time を PImpl で実装 (engine/include/sprout/core/Game.hpp, GameConfig.hpp, Time.hpp, engine/src/core/Game.cpp)
- [ ] T027 [US1] 一括ヘッダを作成し、公開ヘッダに SFML 型が無いことを検査するテストを追加 (engine/include/sprout/Sprout.hpp, tests/unit/PublicHeaderIsolationTests.cpp)
- [ ] T028 [US1] サンプル HelloSprout（30 行以内）を作成 (samples/HelloSprout/main.cpp)
- [ ] T029 [US1] 学生配布テンプレートを作成 (templates/StudentProject/CMakeLists.txt, templates/StudentProject/src/main.cpp)

**Checkpoint**: US1 単独で授業に使える

## Phase 4: User Story 2 - エディタでシーン編集・保存 (P2)

**Independent Test**: 3 オブジェクト配置→保存→再読込で一致、Ctrl+Z で戻る

- [ ] T030 [P] [US2] シーン JSON 往復テスト・版不一致・未知 type のテスト (tests/integration/SceneRoundTripTests.cpp)
- [ ] T031 [P] [US2] PropertyList と BehaviorRegistry のテスト (tests/unit/reflect/PropertyListTests.cpp, tests/unit/reflect/BehaviorRegistryTests.cpp)
- [ ] T032 [P] [US2] CommandHistory と各コマンドの execute/undo/mergeWith のテスト (tests/unit/editor/CommandHistoryTests.cpp, tests/unit/editor/EditorCommandTests.cpp)
- [ ] T033 [US2] Property / PropertyList / BehaviorRegistry / SPROUT_REGISTER_BEHAVIOR を実装 (engine/include/sprout/reflect/Property.hpp, BehaviorRegistry.hpp, engine/src/reflect/BehaviorRegistry.cpp)
- [ ] T034 [US2] SceneSerializer / SceneDeserializer / ComponentCodec を実装 (engine/src/serialization/SceneSerializer.cpp, SceneDeserializer.cpp, ComponentCodec.cpp)
- [ ] T035 [US2] エディタ専用アクセス層を実装 (engine/include/sprout/editor_access/SceneInspector.hpp, engine/src/editor_access/SceneInspector.cpp)
- [ ] T036 [US2] EditorApp / EditorContext / Selection とドッキングレイアウト (editor/src/EditorApp.cpp, editor/src/EditorContext.cpp, editor/src/Selection.cpp, editor/src/main.cpp)
- [ ] T037 [US2] EditorCommand, CommandHistory と 5 コマンドを実装 (editor/src/command/*.cpp)
- [ ] T038 [P] [US2] SceneTreePanel（追加・削除・複製・親子 D&D）(editor/src/panel/SceneTreePanel.cpp)
- [ ] T039 [P] [US2] InspectorPanel（全 Component と Behavior プロパティ編集）(editor/src/panel/InspectorPanel.cpp)
- [ ] T040 [P] [US2] ViewportPanel と TranslateGizmo（選択・ドラッグ・ズーム/パン）(editor/src/panel/ViewportPanel.cpp, editor/src/gizmo/TranslateGizmo.cpp)
- [ ] T041 [P] [US2] AssetBrowserPanel と LogPanel (editor/src/panel/AssetBrowserPanel.cpp, editor/src/panel/LogPanel.cpp)
- [ ] T042 [US2] 保存/開く/新規 と Ctrl+S/Z/Y ショートカット (editor/src/EditorApp.cpp)

## Phase 5: User Story 3 - エディタ内再生 (P3)

- [ ] T043 [P] [US3] PlayModeController のテスト（再生→停止で状態復元、一時停止）(tests/integration/PlayModeTests.cpp)
- [ ] T044 [US3] PlayModeController を実装しツールバーに再生/一時停止/停止を追加 (editor/src/play/PlayModeController.cpp, editor/src/EditorApp.cpp)

## Phase 6: User Story 4 - 当たり判定とタイルマップ (P4)

- [ ] T045 [P] [US4] Aabb と CollisionWorld のテスト（接触開始/終了・めり込み解消・トリガー）(tests/unit/physics/CollisionWorldTests.cpp)
- [ ] T046 [P] [US4] TileMap のテスト（タイル設定・固体判定・JSON 往復）(tests/unit/component/TileMapTests.cpp)
- [ ] T047 [US4] BoxCollider, Aabb, CollisionWorld と Behavior への接触通知を実装 (engine/include/sprout/component/BoxCollider.hpp, engine/src/physics/Aabb.hpp, engine/src/physics/CollisionWorld.cpp)
- [ ] T048 [US4] TileMap の描画・衝突・シリアライズを実装 (engine/include/sprout/component/TileMap.hpp, engine/src/component/TileMap.cpp)
- [ ] T049 [US4] エディタにタイルペイントモードを追加 (editor/src/panel/TilePalettePanel.cpp, editor/src/command/PaintTilesCommand.cpp)
- [ ] T050 [US4] サンプル TileCollector を作成 (samples/TileCollector/main.cpp, samples/TileCollector/assets/level.scene.json)

## Phase 7: User Story 5 - 音 (P5)

- [ ] T051 [P] [US5] AudioMixer のテスト（同時再生数・音量範囲クランプ、デバイス無し環境でスキップ）(tests/unit/audio/AudioMixerTests.cpp)
- [ ] T052 [US5] Audio 公開 API, AudioSource, AudioMixer を実装 (engine/include/sprout/audio/Audio.hpp, engine/include/sprout/component/AudioSource.hpp, engine/src/audio/AudioMixer.cpp)

## Phase 8: Polish

- [ ] T053 [P] 1,000 スプライト 60 fps のベンチマーク (tests/perf/SpriteBenchmark.cpp)
- [ ] T054 [P] 日本語エラーメッセージ一覧の整備とテスト (engine/src/core/ErrorMessages.hpp, tests/unit/core/ErrorMessagesTests.cpp)
- [ ] T055 [P] 学生向け README（導入〜最初のゲームまで）(README.md, docs/getting-started.md)
- [ ] T056 VS2022 / VS2026 両プリセットで build + ctest + sprout_lint を通し quickstart.md を全実施 (specs/001-student-2d-engine/quickstart.md)

## Dependencies & Execution Order
- Phase 1 → Phase 2 → US1 →（US2, US4, US5 は並行可）→ US3 は US2 完了後 → Polish
- 各ストーリー内: テスト → 内部実装 → 公開 API → サンプル

## Parallel Example: Phase 2
```text
T007, T009, T017 を同時に着手（別ファイル・依存なし）
```

## Implementation Strategy
- MVP = Phase 1 + 2 + US1（T001–T029）。ここで一度授業で試用し、API の使いやすさを確認してから US2 へ進む
