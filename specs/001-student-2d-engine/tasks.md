# Tasks: Sprout Engine — 商業水準 2D エンジン + 統合エディタ

**Input**: `specs/001-student-2d-engine/`（spec v2 / plan v2 / er-diagram / data-model / contracts / research / quickstart）
**Tests**: **必須**（FR-902, constitution IV/VI）。各ストーリーはテストタスクを先に置く。テストを書き、失敗を確認してから実装する。
**Format**: `- [ ] Txxx [P?] [USn?] 説明 (パス)`。[P] = 別ファイルかつ未完了タスクに依存せず並行可。
**完了の定義（全タスク共通・constitution VI）**: テスト合格 / `/W4 /WX` 警告ゼロ / `sprout_lint` 違反ゼロ / 公開 API は Doxygen 日本語コメント + 例 / データ構造変更時は er-diagram.md 更新 / レビュー通過

---

## Phase 1: Setup（ビルド基盤・CI・規約）

- [ ] T001 ルート CMake（C++20、オプション SPROUT_BUILD_EDITOR/TESTS/SAMPLES/CLI, SPROUT_MONOLITHIC, SPROUT_WITH_TRACY, SPROUT_ENABLE_ASAN）を作成 (CMakeLists.txt)
- [ ] T002 [P] プリセット vs2022 / vs2026 / ninja-msvc / ci / asan と各 build・test プリセットを作成 (CMakePresets.json)
- [ ] T003 [P] vcpkg マニフェスト（sfml, box2d, imgui[docking-experimental], imgui-sfml, imnodes, nlohmann-json, zstd, xxhash, efsw, nativefiledialog-extended, tracy, gtest）と builtin-baseline 固定 (vcpkg.json)
- [ ] T004 [P] 共通警告設定 `/W4 /permissive- /WX /utf-8 /Zc:__cplusplus` 関数 sprout_set_warnings (cmake/SproutWarnings.cmake)
- [ ] T005 [P] constitution II の命名規則を readability-identifier-naming に設定 (.clang-tidy)
- [ ] T006 [P] 書式規則（4 スペース、120 桁、include 順）を設定 (.clang-format)
- [ ] T007 [P] `sprout_lint` ターゲット（clang-tidy + clang-format --dry-run）を追加 (cmake/SproutLint.cmake)
- [ ] T008 [P] 改行・文字コード規則を設定 (.editorconfig, .gitattributes)
- [ ] T009 [P] 版番号生成（semver + git hash → Version.hpp）(cmake/SproutVersion.cmake, engine/include/sprout/core/Version.hpp.in)
- [ ] T010 内部モジュール作成関数 sprout_add_module(NAME DEPS) と許可依存表による DAG 強制 (cmake/SproutModule.cmake)
- [ ] T011 [P] ASan / コードカバレッジ（OpenCppCoverage）設定関数 (cmake/SproutSanitizers.cmake, cmake/SproutCoverage.cmake)
- [ ] T012 テスト基盤（GoogleTest 取込、ラベル unit/integration/determinism/editor_e2e/perf、sprout_add_test 関数）(tests/CMakeLists.txt, cmake/SproutTesting.cmake)
- [ ] T013 [P] テスト支援（ヘッドレスエンジン Fixture、一時プロジェクト生成、ログ捕捉）(tests/support/HeadlessEngineFixture.hpp, tests/support/TempProject.hpp, tests/support/LogCapture.hpp)
- [ ] T014 [P] CI: windows-2022 で configure/build/ctest/sprout_lint/asan (.github/workflows/ci.yml)
- [ ] T015 [P] CI: VS2026 自前ランナー用ワークフロー (.github/workflows/self-hosted-vs2026.yml)
- [ ] T016 [P] Doxygen 設定（公開ヘッダのみ・日本語・警告をエラー化）と docs ワークフロー (docs/Doxyfile, .github/workflows/docs.yml)
- [ ] T017 [P] 開発規約・完了の定義・ブランチ/コミット規則 (CONTRIBUTING.md)
- [ ] T018 [P] PR テンプレート（DoD チェックリスト・ER 図更新欄）(.github/pull_request_template.md)
- [ ] T019 [P] Issue テンプレート（不具合/機能要望）(.github/ISSUE_TEMPLATE/bug.yml, .github/ISSUE_TEMPLATE/feature.yml)
- [ ] T020 [P] ライセンスと第三者ライセンス表記 (LICENSE, THIRD_PARTY_NOTICES.md)

---

## Phase 2: Foundational（M0: 全ストーリーの前提。完了まで US 着手不可）

### 2-1 core: 数学
- [ ] T021 [P] Vector2 / Vector2Int のテスト（演算・長さ・正規化・内積・外積・回転・比較）(tests/unit/math/Vector2Tests.cpp)
- [ ] T022 [P] Vector2 / Vector2Int を実装 (engine/include/sprout/math/Vector2.hpp)
- [ ] T023 [P] Matrix3（2D アフィン）のテスト（合成・逆行列・点/方向変換）(tests/unit/math/Matrix3Tests.cpp)
- [ ] T024 [P] Matrix3 を実装 (engine/include/sprout/math/Matrix3.hpp, engine/src/core/math/Matrix3.cpp)
- [ ] T025 [P] Rect / Circle のテスト（包含・交差・和・拡張）(tests/unit/math/ShapeTests.cpp)
- [ ] T026 [P] Rect / Circle を実装 (engine/include/sprout/math/Rect.hpp, engine/include/sprout/math/Circle.hpp)
- [ ] T027 [P] Color のテスト（RGBA/HSV/16 進/補間/定数）(tests/unit/math/ColorTests.cpp)
- [ ] T028 [P] Color を実装 (engine/include/sprout/math/Color.hpp, engine/src/core/math/Color.cpp)
- [ ] T029 [P] Angle（Degrees/Radians 強い型）のテスト (tests/unit/math/AngleTests.cpp)
- [ ] T030 [P] Angle を実装 (engine/include/sprout/math/Angle.hpp)
- [ ] T031 [P] Easing 全 30 関数の境界値テスト (tests/unit/math/EasingTests.cpp)
- [ ] T032 [P] Easing を実装 (engine/include/sprout/math/Easing.hpp, engine/src/core/math/Easing.cpp)
- [ ] T033 [P] MathUtil（lerp/inverseLerp/clamp/approxEqual/smoothDamp/moveTowards）のテスト (tests/unit/math/MathUtilTests.cpp)
- [ ] T034 [P] MathUtil を実装 (engine/include/sprout/math/MathUtil.hpp)

### 2-2 core: 基本サービス
- [ ] T035 [P] Result<T> / Error のテスト (tests/unit/core/ResultTests.cpp)
- [ ] T036 [P] Result<T> / Error を実装 (engine/include/sprout/core/Result.hpp)
- [ ] T037 [P] 日本語エラーメッセージ表（コード→何が/どこで/どうすれば）のテスト (tests/unit/core/ErrorCatalogTests.cpp)
- [ ] T038 [P] ErrorCatalog と SPROUT_ASSERT を実装 (engine/src/core/ErrorCatalog.hpp, engine/src/core/ErrorCatalog.cpp, engine/src/core/Assert.hpp)
- [ ] T039 [P] Log（レベル・カテゴリ・std::format・複数シンク・スレッド安全）のテスト (tests/unit/core/LogTests.cpp)
- [ ] T040 [P] Log 公開 API と ConsoleSink / FileSink / CallbackSink を実装 (engine/include/sprout/core/Log.hpp, engine/src/core/log/*.cpp)
- [ ] T041 [P] Guid（生成・文字列往復・ハッシュ）のテスト (tests/unit/core/GuidTests.cpp)
- [ ] T042 [P] Guid を実装 (engine/src/core/Guid.hpp, engine/src/core/Guid.cpp)
- [ ] T043 [P] Hash（xxHash64 ラッパ）と StringId（インターン）のテスト (tests/unit/core/HashTests.cpp)
- [ ] T044 [P] Hash / StringId を実装 (engine/src/core/Hash.hpp, engine/src/core/StringId.cpp)
- [ ] T045 [P] EventBus（型付き発行/購読/解除・所有者破棄で自動解除・発行中の解除安全）のテスト (tests/unit/core/EventBusTests.cpp)
- [ ] T046 [P] EventBus と公開 Event<T> を実装 (engine/src/core/EventBus.cpp, engine/include/sprout/core/Event.hpp)
- [ ] T047 [P] JobSystem（投入・依存・メインスレッド継続・停止）のテスト (tests/unit/core/JobSystemTests.cpp)
- [ ] T048 [P] JobSystem を実装 (engine/src/core/job/JobSystem.cpp, engine/src/core/job/JobHandle.hpp)
- [ ] T049 [P] FrameAllocator / PoolAllocator のテスト (tests/unit/core/AllocatorTests.cpp)
- [ ] T050 [P] FrameAllocator / PoolAllocator を実装 (engine/src/core/memory/FrameAllocator.cpp, engine/src/core/memory/PoolAllocator.hpp)
- [ ] T051 [P] FileSystem（UTF-8/日本語パス・正規化・マウント・ネイティブ/パック切替）のテスト (tests/unit/core/FileSystemTests.cpp)
- [ ] T052 [P] FileSystem と NativeMount を実装 (engine/src/core/fs/FileSystem.cpp, engine/src/core/fs/NativeMount.cpp)
- [ ] T053 [P] Random（シード・範囲・確率・重み付き選択・円内点）のテスト (tests/unit/core/RandomTests.cpp)
- [ ] T054 [P] Random を実装 (engine/include/sprout/core/Random.hpp, engine/src/core/Random.cpp)

### 2-3 platform
- [ ] T055 Window（SFML ラップ: 生成・解像度変更・フルスクリーン・VSync・アイコン）を実装 (engine/src/platform/Window.cpp)
- [ ] T056 [P] SFML イベント→内部 PlatformEvent 変換のテスト (tests/unit/platform/PlatformEventTests.cpp)
- [ ] T057 PlatformEvent 変換を実装 (engine/src/platform/PlatformEventTranslator.cpp)
- [ ] T058 [P] 高 DPI 対応・モニタ情報取得 (engine/src/platform/Display.cpp)
- [ ] T059 [P] クリップボード (engine/src/platform/Clipboard.cpp)
- [ ] T060 [P] 高精度クロック (engine/src/platform/Clock.cpp)
- [ ] T061 [P] UserCodeGuard（SEH __try + std::exception 捕捉 → UserCodeFault）のテスト (tests/unit/platform/UserCodeGuardTests.cpp)
- [ ] T062 UserCodeGuard を実装 (engine/src/platform/UserCodeGuard.cpp)
- [ ] T063 [P] CrashHandler（minidump + ログ退避）を実装 (engine/src/platform/CrashHandler.cpp)

### 2-4 reflection
- [ ] T064 [P] TypeId<T> / TypeInfo / FieldInfo / TypeRegistry（登録・解除・名前検索・モジュール単位解除）のテスト (tests/unit/reflection/TypeRegistryTests.cpp)
- [ ] T065 TypeInfo / FieldInfo / TypeRegistry を実装 (engine/src/reflection/TypeRegistry.cpp, engine/src/reflection/TypeInfo.hpp)
- [ ] T066 [P] PropertyValue（variant・型変換・比較）のテスト (tests/unit/reflection/PropertyValueTests.cpp)
- [ ] T067 PropertyValue を実装 (engine/include/sprout/reflect/PropertyValue.hpp)
- [ ] T068 [P] 属性 Range/Tooltip/Header/Hidden/ReadOnly/Multiline を実装 (engine/include/sprout/reflect/Attributes.hpp)
- [ ] T069 [P] SPROUT_BEHAVIOR / SPROUT_PROPERTY マクロのテスト（登録・オフセット・属性取得）(tests/unit/reflection/RegistrationMacroTests.cpp)
- [ ] T070 SPROUT_BEHAVIOR / SPROUT_PROPERTY を実装 (engine/include/sprout/reflect/Registration.hpp)
- [ ] T071 [P] PropertyPath（"color.r", "items[2]" の get/set）のテスト (tests/unit/reflection/PropertyPathTests.cpp)
- [ ] T072 PropertyPath を実装 (engine/src/reflection/PropertyPath.cpp)

### 2-5 serialization
- [ ] T073 [P] Archive（JsonArchive / BinaryArchive 共通 IF、キー順安定）のテスト (tests/unit/serialization/ArchiveTests.cpp)
- [ ] T074 JsonArchive / BinaryArchive を実装 (engine/src/serialization/JsonArchive.cpp, engine/src/serialization/BinaryArchive.cpp)
- [ ] T075 [P] Migrator 連鎖（版 N→N+1 の順次適用・新版拒否）のテスト (tests/unit/serialization/MigratorTests.cpp)
- [ ] T076 Migrator を実装 (engine/src/serialization/Migrator.cpp)
- [ ] T077 [P] リフレクション駆動シリアライザ（全 PropertyValue 型・配列・参照）のテスト (tests/unit/serialization/ReflectionSerializerTests.cpp)
- [ ] T078 ReflectionSerializer を実装 (engine/src/serialization/ReflectionSerializer.cpp)
- [ ] T079 [P] FileIdMap（fileId ↔ handle 相互変換）のテスト (tests/unit/serialization/FileIdMapTests.cpp)
- [ ] T080 FileIdMap を実装 (engine/src/serialization/FileIdMap.cpp)

### 2-6 scene
- [ ] T081 [P] ObjectTable（スロット・フリーリスト・世代・SoA）のテスト (tests/unit/scene/ObjectTableTests.cpp)
- [ ] T082 ObjectTable / ObjectSlot を実装 (engine/src/scene/ObjectTable.cpp)
- [ ] T083 [P] ComponentPool<T>（密配列・sparse 索引・削除時詰め・世代）のテスト (tests/unit/scene/ComponentPoolTests.cpp)
- [ ] T084 IComponentPool / ComponentPool<T> を実装 (engine/src/scene/ComponentPool.hpp)
- [ ] T085 [P] Hierarchy（親子付替・兄弟順・循環拒否・無効化伝播）のテスト (tests/unit/scene/HierarchyTests.cpp)
- [ ] T086 Hierarchy を実装 (engine/src/scene/Hierarchy.cpp)
- [ ] T087 [P] Transform（ローカル/ワールド・dirty 伝播・座標変換・keepWorldPosition）のテスト (tests/unit/scene/TransformTests.cpp)
- [ ] T088 Transform を実装 (engine/include/sprout/component/Transform.hpp, engine/src/scene/Transform.cpp)
- [ ] T089 [P] PendingQueue（更新中の生成/破棄/追加/削除の反映時点）のテスト (tests/unit/scene/PendingQueueTests.cpp)
- [ ] T090 PendingQueue を実装 (engine/src/scene/PendingQueue.cpp)
- [ ] T091 [P] World（生成・破棄・名前/タグ/型検索・レイヤー・永続指定）のテスト (tests/unit/scene/WorldTests.cpp)
- [ ] T092 World を実装 (engine/src/scene/World.cpp)
- [ ] T093 [P] LifecycleDispatcher（onCreate〜onDestroy 順序・executionOrder・無効時スキップ）のテスト (tests/unit/scene/LifecycleTests.cpp)
- [ ] T094 LifecycleDispatcher を実装 (engine/src/scene/LifecycleDispatcher.cpp)
- [ ] T095 [P] 公開 GameObject / Ref<T> / Scene のテスト（無効ハンドル安全・テンプレート委譲）(tests/unit/scene/PublicSceneApiTests.cpp)
- [ ] T096 公開 Component / GameObject / Ref<T> / Scene を実装 (engine/include/sprout/component/Component.hpp, engine/include/sprout/scene/GameObject.hpp, engine/include/sprout/scene/Ref.hpp, engine/include/sprout/scene/Scene.hpp, engine/src/facade/SceneFacade.cpp)
- [ ] T097 [P] シーン往復テスト（Save→Load→Save バイト一致、Missing Script 保持、新版拒否）(tests/integration/SceneRoundTripTests.cpp)
- [ ] T098 SceneSerializer / SceneDeserializer を実装 (engine/src/serialization/SceneSerializer.cpp, engine/src/serialization/SceneDeserializer.cpp)
- [ ] T099 [P] WorldSnapshot（取得・復元・型差替時のフィールド移行）のテスト (tests/unit/scene/WorldSnapshotTests.cpp)
- [ ] T100 WorldSnapshot を実装 (engine/src/scene/WorldSnapshot.cpp)

### 2-7 時間・フレーム
- [ ] T101 [P] TimeSystem（固定ステップ蓄積・最大 Δt 切捨て・timeScale・unscaled）のテスト (tests/unit/core/TimeSystemTests.cpp)
- [ ] T102 TimeSystem と公開 Time を実装 (engine/src/core/TimeSystem.cpp, engine/include/sprout/core/Time.hpp)
- [ ] T103 [P] Timer（after/every・所有者破棄で停止・unscaled）のテスト (tests/unit/core/TimerTests.cpp)
- [ ] T104 Timer を実装 (engine/include/sprout/core/Timer.hpp, engine/src/core/TimerService.cpp)
- [ ] T105 [P] Coroutine（C++20 Task・waitSeconds/nextFrame/waitUntil/waitFixedUpdate・所有者破棄で中断）のテスト (tests/unit/core/CoroutineTests.cpp)
- [ ] T106 Coroutine / Task を実装 (engine/include/sprout/core/Coroutine.hpp, engine/src/core/CoroutineScheduler.cpp)
- [ ] T107 [P] Engine フレーム順序（plan のフェーズ順）をヘッドレスで検証するテスト (tests/integration/FrameOrderTests.cpp)
- [ ] T108 Engine / FrameScheduler を実装 (engine/src/core/Engine.cpp, engine/src/core/FrameScheduler.cpp)

### 2-8 asset（基盤）
- [ ] T109 [P] AssetDatabase（走査・.meta 生成・移動/改名で GUID 不変・重複 GUID 検出）のテスト (tests/unit/asset/AssetDatabaseTests.cpp)
- [ ] T110 AssetDatabase / AssetMeta を実装 (engine/src/asset/AssetDatabase.cpp, engine/src/asset/AssetMeta.cpp)
- [ ] T111 [P] AssetManager（参照カウント・解放・失敗時フォールバック・型不一致拒否）のテスト (tests/unit/asset/AssetManagerTests.cpp)
- [ ] T112 AssetManager / AssetSlot を実装 (engine/src/asset/AssetManager.cpp)
- [ ] T113 [P] 公開 AssetRef<T> / Assets のテスト (tests/unit/asset/AssetRefTests.cpp)
- [ ] T114 公開 AssetRef<T> / Assets を実装 (engine/include/sprout/asset/AssetRef.hpp, engine/include/sprout/asset/Assets.hpp, engine/src/facade/AssetsFacade.cpp)
- [ ] T115 Importer IF と ImporterRegistry を実装 (engine/src/asset/Importer.hpp, engine/src/asset/ImporterRegistry.cpp)
- [ ] T116 [P] TextureImporter（フィルタ・PPU・ピボット）と Texture/Sprite のテスト (tests/unit/asset/TextureImporterTests.cpp)
- [ ] T117 TextureImporter / Texture / Sprite を実装 (engine/src/asset/importers/TextureImporter.cpp, engine/include/sprout/asset/Texture.hpp, engine/include/sprout/asset/Sprite.hpp)
- [ ] T118 [P] FontImporter / AudioImporter のテスト (tests/unit/asset/FontAudioImporterTests.cpp)
- [ ] T119 FontImporter / AudioImporter と Font / AudioClip を実装 (engine/src/asset/importers/FontImporter.cpp, engine/src/asset/importers/AudioImporter.cpp)
- [ ] T120 [P] FallbackAssets（チェッカー画像・無音・内蔵フォント）を実装 (engine/src/asset/FallbackAssets.cpp)
- [ ] T121 [P] 非同期読込（JobSystem 経由・進捗）のテスト (tests/unit/asset/AsyncLoadTests.cpp)
- [ ] T122 非同期読込と AsyncOperation<T> を実装 (engine/src/asset/AsyncLoader.cpp, engine/include/sprout/asset/AsyncOperation.hpp)
- [ ] T123 [P] ファイル監視と自動再インポート（使用中アセット差替）のテスト (tests/integration/AssetHotReloadTests.cpp)
- [ ] T124 AssetWatcher（efsw）と再インポートを実装 (engine/src/asset/AssetWatcher.cpp)
- [ ] T125 [P] 依存関係索引（参照元/参照先検索）のテスト (tests/unit/asset/AssetDependencyTests.cpp)
- [ ] T126 AssetDependencyIndex を実装 (engine/src/asset/AssetDependencyIndex.cpp)

### 2-9 DLL 化と境界検査
- [ ] T127 SproutEngine 共有ライブラリ（内部 static を結合、SPROUT_API 輸出）を作成 (engine/CMakeLists.txt, engine/include/sprout/core/Export.hpp)
- [ ] T128 [P] 公開ヘッダ隔離テスト（include グラフに SFML/box2d/imgui/nlohmann/detail が無い）(tests/unit/PublicHeaderIsolationTests.cpp)
- [ ] T129 [P] モジュール依存 DAG 検査テスト (tests/unit/ModuleDependencyTests.cpp)
- [ ] T130 一括インクルード (engine/include/sprout/Sprout.hpp)

**Checkpoint (M0)**: ヘッドレスでワールド・オブジェクト・コンポーネント・シリアライズ・アセットがテストで動き、CI 緑

---
## Phase 3: User Story 1 - コードだけで最初のゲームを動かす (P1) 🎯 MVP

**Goal**: 30 行以内で画像表示とキー移動 / **Independent Test**: `01_HelloSprout` を WASD で移動、画像欠損で代替 + 日本語エラー

### Tests
- [ ] T131 [P] [US1] RenderQueue ソートキー（レイヤー/順序/マテリアル/テクスチャ）のテスト (tests/unit/render/RenderQueueTests.cpp)
- [ ] T132 [P] [US1] SpriteBatcher（同一テクスチャ連続区間の結合・頂点生成）のテスト (tests/unit/render/SpriteBatcherTests.cpp)
- [ ] T133 [P] [US1] Camera（ワールド⇔画面変換・ズーム・ビューポート・複数カメラ深度）のテスト (tests/unit/render/CameraTests.cpp)
- [ ] T134 [P] [US1] 画面外カリングのテスト (tests/unit/render/CullingTests.cpp)
- [ ] T135 [P] [US1] InputDevice 状態（押下瞬間/継続/解放、フレーム境界）のテスト (tests/unit/input/InputDeviceStateTests.cpp)
- [ ] T136 [P] [US1] InputAction（Button/Axis1D/Axis2D・WASD 合成・不感帯）のテスト (tests/unit/input/InputActionTests.cpp)
- [ ] T137 [P] [US1] Game ファサードをヘッドレスで 120 フレーム回す統合テスト (tests/integration/GameFacadeTests.cpp)

### Implementation
- [ ] T138 [US1] RenderSystem（収集・基数ソート・カメラ毎描画）を実装 (engine/src/render/RenderSystem.cpp)
- [ ] T139 [US1] SpriteBatcher を実装 (engine/src/render/SpriteBatcher.cpp)
- [ ] T140 [P] [US1] Camera コンポーネント（背景色・ズーム・ビューポート・cullingMask・深度）を実装 (engine/include/sprout/render/Camera.hpp, engine/src/render/Camera.cpp)
- [ ] T141 [P] [US1] SpriteRenderer（Simple・色・反転・描画レイヤー・順序）を実装 (engine/include/sprout/render/SpriteRenderer.hpp, engine/src/render/SpriteRenderer.cpp)
- [ ] T142 [P] [US1] TextRenderer（TTF・日本語・揃え・折返し）を実装 (engine/include/sprout/render/TextRenderer.hpp, engine/src/render/TextRenderer.cpp)
- [ ] T143 [P] [US1] ShapeRenderer（線・矩形・円・多角形）を実装 (engine/include/sprout/render/ShapeRenderer.hpp, engine/src/render/ShapeRenderer.cpp)
- [ ] T144 [P] [US1] 既定マテリアル / シェーダ（スプライト用）を実装 (engine/src/render/DefaultMaterials.cpp)
- [ ] T145 [US1] カリングを実装 (engine/src/render/Culling.cpp)
- [ ] T146 [US1] InputSystem（キーボード/マウス・フレーム状態）を実装 (engine/src/input/InputSystem.cpp, engine/src/input/KeyboardDevice.cpp, engine/src/input/MouseDevice.cpp)
- [ ] T147 [US1] InputActionAsset 読込とアクション解決を実装 (engine/src/input/ActionMapRuntime.cpp, engine/src/input/BindingResolver.cpp)
- [ ] T148 [P] [US1] 既定アクションマップ（Move/Jump/Fire/Submit/Cancel/Pause）を作成 (engine/resources/DefaultInputActions.inputactions)
- [ ] T149 [US1] 公開 Input / InputAction / Key / MouseButton を実装 (engine/include/sprout/input/Input.hpp, engine/include/sprout/input/InputAction.hpp, engine/include/sprout/input/Key.hpp, engine/include/sprout/input/MouseButton.hpp)
- [ ] T150 [US1] Behavior 基底（ライフサイクル仮想関数・コンテキストアクセサ）を実装 (engine/include/sprout/component/Behavior.hpp, engine/src/facade/BehaviorContext.cpp)
- [ ] T151 [US1] Game / GameConfig（コードのみ起動経路）を実装 (engine/include/sprout/app/Game.hpp, engine/include/sprout/app/GameConfig.hpp, engine/src/facade/Game.cpp)
- [ ] T152 [P] [US1] 画面上エラーオーバーレイ（日本語、開発ビルドのみ）を実装 (engine/src/diagnostics/ErrorOverlay.cpp)
- [ ] T153 [US1] サンプル 01_HelloSprout（30 行以内）(samples/01_HelloSprout/main.cpp, samples/01_HelloSprout/Assets/)
- [ ] T154 [P] [US1] 性能テスト: スプライト 10,000 個 60 fps (tests/perf/SpriteStressTests.cpp)
- [ ] T155 [P] [US1] チュートリアル 1 章「環境構築」・2 章「最初のゲーム」(docs/tutorial/01-setup.md, docs/tutorial/02-first-game.md)

**Checkpoint**: US1 単独で授業に使える

---

## Phase 4: User Story 2 - プロジェクト作成とエディタでのシーン編集 (P1)

**Goal**: ハブ→作成→配置→保存→再起動で復元 / **Independent Test**: quickstart #2

### Tests（editor_core はヘッドレス）
- [ ] T156 [P] [US2] ProjectDescriptor 作成/開く/最近一覧のテスト (tests/unit/editor/ProjectServiceTests.cpp)
- [ ] T157 [P] [US2] CommandHistory（execute/undo/redo・mergeKey 統合・保存点・容量 500）のテスト (tests/unit/editor/CommandHistoryTests.cpp)
- [ ] T158 [P] [US2] Create/Delete/Duplicate コマンドのテスト（参照の復元含む）(tests/unit/editor/ObjectCommandTests.cpp)
- [ ] T159 [P] [US2] SetProperty / Reparent / Reorder コマンドのテスト (tests/unit/editor/PropertyCommandTests.cpp)
- [ ] T160 [P] [US2] Add/RemoveComponent コマンドのテスト (tests/unit/editor/ComponentCommandTests.cpp)
- [ ] T161 [P] [US2] CompositeCommand（複数選択一括）のテスト (tests/unit/editor/CompositeCommandTests.cpp)
- [ ] T162 [P] [US2] Selection（複数・アクティブ・文脈切替・破棄追従）のテスト (tests/unit/editor/SelectionTests.cpp)
- [ ] T163 [P] [US2] オブジェクトのコピー＆ペースト（内部参照の付け替え）のテスト (tests/unit/editor/ObjectClipboardTests.cpp)
- [ ] T164 [P] [US2] SceneDocument dirty 判定と保存確認フローのテスト (tests/unit/editor/SceneDocumentTests.cpp)
- [ ] T165 [P] [US2] 自動保存とクラッシュ復旧のテスト (tests/unit/editor/AutosaveTests.cpp)
- [ ] T166 [P] [US2] ギズモ数学（移動/回転/拡大/矩形・スナップ・ローカル/グローバル）のテスト (tests/unit/editor/GizmoMathTests.cpp)
- [ ] T167 [P] [US2] シーンビューのピッキング（クリック・矩形選択・重なり順）のテスト (tests/unit/editor/PickingTests.cpp)

### Implementation: editor_core
- [ ] T168 [US2] sprout_editor_core ターゲットとエディタ専用アクセス層を作成 (editor/core/CMakeLists.txt, engine/include_editor/sprout/editor_access/WorldAccess.hpp)
- [ ] T169 [US2] ProjectService（作成・開く・最近一覧・テンプレート展開）を実装 (editor/core/src/project/ProjectService.cpp)
- [ ] T170 [P] [US2] テンプレート 2DEmpty（フォルダ・設定・空シーン・Game/ CMake）を作成 (templates/2DEmpty/)
- [ ] T171 [US2] EditorSession / SceneDocument を実装 (editor/core/src/document/EditorSession.cpp, editor/core/src/document/SceneDocument.cpp)
- [ ] T172 [US2] EditorCommand / CommandHistory を実装 (editor/core/src/command/EditorCommand.hpp, editor/core/src/command/CommandHistory.cpp)
- [ ] T173 [P] [US2] CreateObject / DeleteObjects / DuplicateObjects コマンドを実装 (editor/core/src/command/ObjectCommands.cpp)
- [ ] T174 [P] [US2] SetProperty / Reparent / Reorder コマンドを実装 (editor/core/src/command/PropertyCommands.cpp, editor/core/src/command/HierarchyCommands.cpp)
- [ ] T175 [P] [US2] AddComponent / RemoveComponent / CompositeCommand を実装 (editor/core/src/command/ComponentCommands.cpp, editor/core/src/command/CompositeCommand.cpp)
- [ ] T176 [US2] Selection を実装 (editor/core/src/selection/Selection.cpp)
- [ ] T177 [US2] ObjectClipboard を実装 (editor/core/src/selection/ObjectClipboard.cpp)
- [ ] T178 [US2] AutosaveService / CrashRecovery を実装 (editor/core/src/document/AutosaveService.cpp)
- [ ] T179 [P] [US2] EditorPreferences / ShortcutMap（保存・既定値）を実装 (editor/core/src/prefs/EditorPreferences.cpp, editor/core/src/prefs/ShortcutMap.cpp)
- [ ] T180 [P] [US2] GizmoMath / Picking を実装 (editor/core/src/scene/GizmoMath.cpp, editor/core/src/scene/Picking.cpp)

### Implementation: エディタ UI
- [ ] T181 [US2] EditorApplication（ImGui-SFML 初期化・docking・メインループ）を実装 (editor/app/src/EditorApplication.cpp, editor/app/src/main.cpp)
- [ ] T182 [P] [US2] 日本語フォント（Noto Sans JP）・Dark/Light テーマ・フォント倍率 (editor/app/src/theme/EditorTheme.cpp, editor/app/fonts/)
- [ ] T183 [P] [US2] EditorWindow 基底とウィンドウ登録・レイアウト保存/復元/既定化 (editor/app/src/window/EditorWindow.hpp, editor/app/src/window/WindowManager.cpp)
- [ ] T184 [US2] ProjectHub ウィンドウ (editor/app/src/window/ProjectHubWindow.cpp)
- [ ] T185 [US2] メインメニュー・ツールバー・ショートカット処理 (editor/app/src/MainMenuBar.cpp, editor/app/src/Toolbar.cpp)
- [ ] T186 [US2] SceneView（RenderTexture 描画・パン/ズーム・グリッド・選択強調）(editor/app/src/window/SceneViewWindow.cpp)
- [ ] T187 [P] [US2] 移動/回転/拡大/矩形ギズモとスナップ設定 UI (editor/app/src/gizmo/TransformGizmo.cpp, editor/app/src/gizmo/RectGizmo.cpp)
- [ ] T188 [P] [US2] GameView（解像度/アスペクト・拡大・統計）(editor/app/src/window/GameViewWindow.cpp)
- [ ] T189 [P] [US2] Hierarchy（ツリー・D&D 親子/並替・検索・有効/ロック/表示切替）(editor/app/src/window/HierarchyWindow.cpp)
- [ ] T190 [US2] Inspector 枠組み（コンポーネント一覧・追加メニュー検索・コピー/リセット/削除/並替・複数選択共通値）(editor/app/src/window/InspectorWindow.cpp)
- [ ] T191 [P] [US2] PropertyDrawer 群（int/float/bool/string/color/vec2/enum/asset/object/component/array）(editor/app/src/inspector/PropertyDrawers.cpp)
- [ ] T192 [P] [US2] 組込みコンポーネント用インスペクタ（Transform/SpriteRenderer/TextRenderer/ShapeRenderer/Camera）(editor/app/src/inspector/BuiltinInspectors.cpp)
- [ ] T193 [P] [US2] AssetBrowser（フォルダツリー・サムネイル・検索・フィルタ・作成/改名/移動/ゴミ箱削除・外部で開く・シーンへ D&D）(editor/app/src/window/AssetBrowserWindow.cpp, editor/app/src/window/ThumbnailCache.cpp)
- [ ] T194 [P] [US2] インポート設定インスペクタ（Texture/Audio/Font）(editor/app/src/inspector/ImportSettingsInspector.cpp)
- [ ] T195 [P] [US2] Console（レベル/カテゴリ絞込・集約・検索・発生元選択）(editor/app/src/window/ConsoleWindow.cpp)
- [ ] T196 [P] [US2] Undo 履歴ウィンドウ（任意時点へ移動）(editor/app/src/window/UndoHistoryWindow.cpp)
- [ ] T197 [P] [US2] 未保存確認・ファイルダイアログ（nativefiledialog）(editor/app/src/dialog/UnsavedChangesDialog.cpp, editor/app/src/dialog/FileDialogs.cpp)
- [ ] T198 [P] [US2] ProjectSettings ウィンドウ（タグ/レイヤー/描画レイヤー/時間/品質）(editor/app/src/window/ProjectSettingsWindow.cpp)
- [ ] T199 [P] [US2] Preferences ウィンドウ（テーマ・言語・ショートカット・自動保存間隔）(editor/app/src/window/PreferencesWindow.cpp)
- [ ] T200 [P] [US2] エディタ UI 文言の日英リソース (editor/app/resources/strings.ja.json, editor/app/resources/strings.en.json)

### E2E / 性能
- [ ] T201 [US2] ImGui Test Engine 組込みと E2E ランナー (tests/editor_e2e/CMakeLists.txt, tests/editor_e2e/E2eMain.cpp)
- [ ] T202 [P] [US2] E2E: ハブで作成→3 体配置→保存→再起動→一致 (tests/editor_e2e/ProjectCreateAndSaveTests.cpp)
- [ ] T203 [P] [US2] E2E: ギズモ操作とインスペクタ同期・スナップ (tests/editor_e2e/GizmoInspectorSyncTests.cpp)
- [ ] T204 [P] [US2] E2E: 複数選択コピペ・Undo 200 手 (tests/editor_e2e/MultiSelectUndoTests.cpp)
- [ ] T205 [P] [US2] E2E: 未保存確認 (tests/editor_e2e/UnsavedPromptTests.cpp)
- [ ] T206 [P] [US2] 性能: 1,000 オブジェクトのシーンを 2 秒以内に開く・操作反映 100 ms (tests/perf/EditorLoadPerfTests.cpp)
- [ ] T207 [P] [US2] チュートリアル 3 章「エディタの基本」(docs/tutorial/03-editor-basics.md)

---

## Phase 5: User Story 3 - C++ で振る舞いを書き、エディタで再生 (P1)

**Goal**: ビルド→再起動なし反映→再生→停止で復元 / **Independent Test**: quickstart #3, #4

### Tests
- [ ] T208 [P] [US3] ModuleHost（ABI 検査・Debug/Release 不一致拒否・シャドウコピー）のテスト (tests/unit/editor/ModuleHostTests.cpp)
- [ ] T209 [P] [US3] ホットリロード（スナップショット→差替→復元・フィールド移行・失敗時前版維持）の統合テスト (tests/integration/HotReloadTests.cpp)
- [ ] T210 [P] [US3] PlayModeController（再生/一時停止/コマ送り/停止復元・値保持指定）のテスト (tests/integration/PlayModeTests.cpp)
- [ ] T211 [P] [US3] 利用者コード例外/アクセス違反→再生停止＋復元のテスト (tests/integration/UserCodeFaultTests.cpp)
- [ ] T212 [P] [US3] BuildService（vswhere 検出・cmake 起動・MSVC 診断解析）のテスト (tests/unit/editor/BuildServiceTests.cpp)
- [ ] T213 [P] [US3] Behavior 公開 API 全ライフサイクルとコンテキストのテスト (tests/unit/scene/BehaviorApiTests.cpp)

### Implementation
- [ ] T214 [US3] sprout_add_game_module 関数（DLL・PDB 別名・ABI 定義）(cmake/SproutGameModule.cmake)
- [ ] T215 [US3] SPROUT_GAME_MODULE と ModuleExports / ModuleHostInfo を実装 (engine/include/sprout/app/GameModule.hpp, engine/src/reflection/ModuleRegistration.cpp)
- [ ] T216 [US3] ModuleHost（シャドウコピー・LoadLibrary・型登録/解除）を実装 (editor/core/src/modulehost/ModuleHost.cpp)
- [ ] T217 [US3] HotReloadCoordinator（監視→待機→スナップショット→差替→復元）を実装 (editor/core/src/modulehost/HotReloadCoordinator.cpp)
- [ ] T218 [US3] BuildService と CompileDiagnosticParser を実装 (editor/core/src/build/BuildService.cpp, editor/core/src/build/CompileDiagnosticParser.cpp)
- [ ] T219 [US3] PlayModeController（WorldSnapshot・Stepping・値保持）を実装 (editor/core/src/playmode/PlayModeController.cpp)
- [ ] T220 [US3] UserCodeGuard をライフサイクル呼出し全経路に適用 (engine/src/scene/LifecycleDispatcher.cpp)
- [ ] T221 [P] [US3] ツールバー再生制御と再生中の UI 着色 (editor/app/src/Toolbar.cpp, editor/app/src/theme/PlayModeTint.cpp)
- [ ] T222 [P] [US3] コンパイルエラー一覧・ビルド状態表示・IDE で開く (editor/app/src/window/CompileErrorsWindow.cpp, editor/app/src/build/IdeLauncher.cpp)
- [ ] T223 [P] [US3] 属性付きプロパティ描画（Range スライダー・Tooltip・Header・Hidden・ReadOnly・Multiline）(editor/app/src/inspector/AttributeDrawers.cpp)
- [ ] T224 [P] [US3] 再生中インスペクタ即時反映 (editor/app/src/window/InspectorWindow.cpp)
- [ ] T225 [P] [US3] テンプレート Game/ に Behavior 雛形と注意書き（static 状態・スレッド禁止）(templates/2DEmpty/Game/src/ExampleBehavior.cpp)
- [ ] T226 [P] [US3] E2E: Behavior 追加→ビルド→インスペクタ表示→再生→停止復元 (tests/editor_e2e/HotReloadPlayTests.cpp)
- [ ] T227 [P] [US3] 性能: 変更→再生まで 10 秒以内（増分ビルド）(tests/perf/IterationTimeTests.cpp)
- [ ] T228 [P] [US3] チュートリアル 4 章「振る舞いを書く」(docs/tutorial/04-behaviors.md)

---

## Phase 6: User Story 4 - アニメーション (P2)

### Tests
- [ ] T229 [P] [US4] スプライト切出し（グリッド/手動/ピボット）のテスト (tests/unit/asset/SpriteSlicingTests.cpp)
- [ ] T230 [P] [US4] SpriteAtlas パッキング（詰込み・余白・最大サイズ）のテスト (tests/unit/asset/SpriteAtlasTests.cpp)
- [ ] T231 [P] [US4] キーフレーム補間（定数/線形/ベジェ）のテスト (tests/unit/animation/KeyframeTests.cpp)
- [ ] T232 [P] [US4] AnimationClip 再生（スプライトトラック・プロパティトラック・ループ）のテスト (tests/unit/animation/ClipPlayerTests.cpp)
- [ ] T233 [P] [US4] Animator 状態機械（パラメータ・条件・exitTime・AnyState・サブマシン・レイヤー）のテスト (tests/unit/animation/AnimatorTests.cpp)
- [ ] T234 [P] [US4] アニメーションイベント発火のテスト (tests/unit/animation/AnimationEventTests.cpp)
- [ ] T235 [P] [US4] Tween（イージング・遅延・ループ・ヨーヨー・連結/並列・所有者破棄）のテスト (tests/unit/animation/TweenTests.cpp)
- [ ] T236 [P] [US4] .anim / .animator 往復テスト (tests/integration/AnimationAssetRoundTripTests.cpp)

### Implementation
- [ ] T237 [US4] スプライト切出しを TextureImporter に追加 (engine/src/asset/importers/TextureImporter.cpp)
- [ ] T238 [P] [US4] SpriteAtlas アセットとパッカー (engine/src/asset/SpriteAtlasPacker.cpp)
- [ ] T239 [US4] AnimationClipData とインポーター (engine/src/animation/AnimationClipData.cpp, engine/src/asset/importers/AnimationClipImporter.cpp)
- [ ] T240 [US4] ClipPlayer を実装 (engine/src/animation/ClipPlayer.cpp)
- [ ] T241 [US4] AnimatorGraph / AnimatorRuntime を実装 (engine/src/animation/AnimatorGraph.cpp, engine/src/animation/AnimatorRuntime.cpp)
- [ ] T242 [US4] 公開 Animator コンポーネント (engine/include/sprout/animation/Animator.hpp, engine/src/animation/Animator.cpp)
- [ ] T243 [US4] TweenEngine と公開 Tween/Sequence/Parallel (engine/src/animation/TweenEngine.cpp, engine/include/sprout/animation/Tween.hpp)
- [ ] T244 [P] [US4] SpriteEditor ウィンドウ（切出し・ピボット）(editor/app/src/window/SpriteEditorWindow.cpp)
- [ ] T245 [P] [US4] AnimationEditor（タイムライン・キー・カーブ・プレビュー・録画モード）(editor/app/src/window/AnimationEditorWindow.cpp, editor/app/src/window/CurveEditor.cpp)
- [ ] T246 [P] [US4] AnimatorGraph エディタ（imnodes: 状態・遷移・パラメータ・条件）(editor/app/src/window/AnimatorGraphWindow.cpp)
- [ ] T247 [P] [US4] アニメーション編集コマンド（Undo 対応）(editor/core/src/command/AnimationCommands.cpp)
- [ ] T248 [P] [US4] Animator インスペクタ（実行時の状態表示）(editor/app/src/inspector/AnimatorInspector.cpp)
- [ ] T249 [P] [US4] E2E: シートから切出し→クリップ作成→状態遷移再生 (tests/editor_e2e/AnimationWorkflowTests.cpp)
- [ ] T250 [P] [US4] チュートリアル 5 章「アニメーション」(docs/tutorial/05-animation.md)

---

## Phase 7: User Story 5 - 物理 (P2)

### Tests
- [ ] T251 [P] [US5] PhysicsWorld（固定ステップ・重力・スリープ・描画補間）のテスト (tests/unit/physics/PhysicsWorldTests.cpp)
- [ ] T252 [P] [US5] Rigidbody2D（種類切替・力・速度・回転固定・gravityScale）のテスト (tests/unit/physics/Rigidbody2DTests.cpp)
- [ ] T253 [P] [US5] 各 Collider 形状（箱/円/カプセル/多角形/辺/複合・オフセット・トリガー）のテスト (tests/unit/physics/ColliderTests.cpp)
- [ ] T254 [P] [US5] 接触イベント（Enter/Stay/Exit・トリガー・接触点/法線・破棄時 Exit）のテスト (tests/unit/physics/ContactEventTests.cpp)
- [ ] T255 [P] [US5] 衝突レイヤー行列のテスト (tests/unit/physics/CollisionMatrixTests.cpp)
- [ ] T256 [P] [US5] クエリ（レイキャスト/形状キャスト/点・範囲検索・LayerMask）のテスト (tests/unit/physics/PhysicsQueryTests.cpp)
- [ ] T257 [P] [US5] ジョイント 6 種のテスト (tests/unit/physics/JointTests.cpp)
- [ ] T258 [P] [US5] 連続衝突検出（高速弾が薄壁を抜けない）のテスト (tests/unit/physics/ContinuousCollisionTests.cpp)
- [ ] T259 [P] [US5] 決定性: 同一入力 1,000 ステップで状態ハッシュ一致 (tests/determinism/PhysicsDeterminismTests.cpp)
- [ ] T260 [P] [US5] 簡易重なり判定（物理なし）のテスト (tests/unit/physics/SimpleOverlapTests.cpp)

### Implementation
- [ ] T261 [US5] PhysicsWorld（Box2D v3 ラップ・固定ステップ・補間）(engine/src/physics/PhysicsWorld.cpp)
- [ ] T262 [US5] BodyProxy 同期（Transform ⇔ body）(engine/src/physics/BodyProxy.cpp)
- [ ] T263 [US5] 公開 Rigidbody2D (engine/include/sprout/physics/Rigidbody2D.hpp, engine/src/physics/Rigidbody2D.cpp)
- [ ] T264 [P] [US5] Collider2D 基底と Box/Circle/Capsule (engine/include/sprout/physics/Collider2D.hpp, engine/include/sprout/physics/BoxCollider2D.hpp, engine/include/sprout/physics/CircleCollider2D.hpp, engine/include/sprout/physics/CapsuleCollider2D.hpp, engine/src/physics/Colliders.cpp)
- [ ] T265 [P] [US5] Polygon / Edge / Composite Collider（凸分解含む）(engine/include/sprout/physics/PolygonCollider2D.hpp, engine/include/sprout/physics/EdgeCollider2D.hpp, engine/src/physics/PolygonDecomposer.cpp)
- [ ] T266 [P] [US5] PhysicsMaterial アセット (engine/src/physics/PhysicsMaterialData.cpp)
- [ ] T267 [US5] CollisionMatrix とフィルタ (engine/src/physics/CollisionFilter.cpp)
- [ ] T268 [US5] ContactDispatcher（Collision 構造体・Behavior 通知）(engine/src/physics/ContactDispatcher.cpp, engine/include/sprout/physics/Collision.hpp)
- [ ] T269 [US5] 公開 Physics2D クエリ (engine/include/sprout/physics/Physics2D.hpp, engine/src/physics/PhysicsQueries.cpp)
- [ ] T270 [P] [US5] Joint2D 6 種 (engine/include/sprout/physics/Joint2D.hpp, engine/src/physics/Joints.cpp)
- [ ] T271 [P] [US5] 簡易重なり判定 API (engine/src/physics/SimpleOverlap.cpp)
- [ ] T272 [P] [US5] コライダー編集ギズモ（頂点編集含む）(editor/app/src/gizmo/ColliderGizmo.cpp)
- [ ] T273 [P] [US5] 物理設定・レイヤー行列 UI (editor/app/src/window/PhysicsSettingsPanel.cpp)
- [ ] T274 [P] [US5] 物理形状の可視化切替 (engine/src/diagnostics/PhysicsDebugView.cpp)
- [ ] T275 [P] [US5] 性能: 動的剛体 1,000 個 60 fps (tests/perf/PhysicsStressTests.cpp)
- [ ] T276 [P] [US5] チュートリアル 6 章「物理」(docs/tutorial/06-physics.md)

---

## Phase 8: User Story 6 - Prefab (P2)

### Tests
- [ ] T277 [P] [US6] Prefab 往復と実行時生成のテスト (tests/unit/scene/PrefabInstantiateTests.cpp)
- [ ] T278 [P] [US6] 上書き記録（値/追加/削除）と変更伝播のテスト (tests/integration/PrefabOverrideTests.cpp)
- [ ] T279 [P] [US6] 入れ子・派生 Prefab のテスト (tests/integration/NestedPrefabTests.cpp)
- [ ] T280 [P] [US6] 循環検出・孤立上書き保持のテスト (tests/unit/scene/PrefabValidationTests.cpp)

### Implementation
- [ ] T281 [US6] PrefabData / PrefabInstanceData とシリアライズ (engine/src/scene/prefab/PrefabData.cpp)
- [ ] T282 [US6] PrefabExpander（展開・上書き適用・入れ子・派生）(engine/src/scene/prefab/PrefabExpander.cpp)
- [ ] T283 [US6] OverrideTracker（差分算出）(engine/src/scene/prefab/OverrideTracker.cpp)
- [ ] T284 [US6] PrefabGraphValidator（DAG 検査）(engine/src/scene/prefab/PrefabGraphValidator.cpp)
- [ ] T285 [US6] 公開 Prefab / instantiate (engine/include/sprout/scene/Prefab.hpp, engine/src/facade/PrefabFacade.cpp)
- [ ] T286 [P] [US6] Prefab 作成（D&D）・Prefab 編集モード (editor/core/src/document/PrefabDocument.cpp, editor/app/src/window/PrefabModeBar.cpp)
- [ ] T287 [P] [US6] 上書きの強調表示と適用/破棄コマンド (editor/core/src/command/PrefabCommands.cpp, editor/app/src/inspector/OverrideIndicators.cpp)
- [ ] T288 [P] [US6] E2E: 敵 Prefab 変更が全インスタンスに反映・上書き保持 (tests/editor_e2e/PrefabWorkflowTests.cpp)
- [ ] T289 [P] [US6] チュートリアル 7 章「Prefab」(docs/tutorial/07-prefab.md)

---

## Phase 9: User Story 7 - タイルマップ (P2)

### Tests
- [ ] T290 [P] [US7] Tileset / Tilemap 多レイヤーデータと往復のテスト (tests/unit/tilemap/TilemapDataTests.cpp)
- [ ] T291 [P] [US7] RuleTile 評価（8 近傍）のテスト (tests/unit/tilemap/RuleTileTests.cpp)
- [ ] T292 [P] [US7] タイル当たり判定の結合（引っかかり無し）のテスト (tests/unit/tilemap/TileColliderMergeTests.cpp)
- [ ] T293 [P] [US7] チャンク描画とアニメタイルのテスト (tests/unit/tilemap/TilemapRenderTests.cpp)
- [ ] T294 [P] [US7] PaintTilesCommand（ブラシ/矩形/塗潰し/消去）のテスト (tests/unit/editor/PaintTilesCommandTests.cpp)

### Implementation
- [ ] T295 [US7] Tileset / RuleTile アセットとインポーター (engine/src/tilemap/TilesetData.cpp, engine/src/tilemap/RuleTileData.cpp)
- [ ] T296 [US7] 公開 Tilemap と TileLayer (engine/include/sprout/tilemap/Tilemap.hpp, engine/src/tilemap/Tilemap.cpp)
- [ ] T297 [US7] TilemapRenderer（チャンク・アニメタイル）(engine/src/tilemap/TilemapRenderer.cpp)
- [ ] T298 [US7] RuleTileEvaluator (engine/src/tilemap/RuleTileEvaluator.cpp)
- [ ] T299 [US7] TileColliderBuilder（矩形結合→Edge/Composite）(engine/src/tilemap/TileColliderBuilder.cpp)
- [ ] T300 [P] [US7] TilePalette ウィンドウとブラシ群・スポイト (editor/app/src/window/TilePaletteWindow.cpp, editor/core/src/tile/TileBrushes.cpp)
- [ ] T301 [P] [US7] PaintTilesCommand (editor/core/src/command/PaintTilesCommand.cpp)
- [ ] T302 [P] [US7] E2E: タイル塗り→保存→再生で衝突 (tests/editor_e2e/TilemapWorkflowTests.cpp)
- [ ] T303 [US7] サンプル 03_Platformer（US4/5/6/7 統合）(samples/03_Platformer/)
- [ ] T304 [P] [US7] チュートリアル 8 章「タイルマップでステージ」(docs/tutorial/08-tilemap.md)

---
## Phase 10: User Story 8 - UI とシーン遷移 (P2)

### Tests
- [ ] T305 [P] [US8] Canvas 拡大方式（固定/基準解像度拡大/画面幅高さ一致）のテスト (tests/unit/ui/CanvasScalerTests.cpp)
- [ ] T306 [P] [US8] RectLayout（アンカー・ピボット・余白・解像度変更）のテスト (tests/unit/ui/RectLayoutTests.cpp)
- [ ] T307 [P] [US8] UI ヒットテストとイベント伝播のテスト (tests/unit/ui/UiEventSystemTests.cpp)
- [ ] T308 [P] [US8] Button / Toggle / Slider / ProgressBar のテスト (tests/unit/ui/BasicWidgetTests.cpp)
- [ ] T309 [P] [US8] InputField（IME・選択・カーソル）/ Dropdown / ScrollView / Mask のテスト (tests/unit/ui/AdvancedWidgetTests.cpp)
- [ ] T310 [P] [US8] LayoutGroup（縦/横/グリッド・内容サイズ）のテスト (tests/unit/ui/LayoutGroupTests.cpp)
- [ ] T311 [P] [US8] フォーカスナビゲーション（キー/パッド・順序指定）のテスト (tests/unit/ui/UiNavigationTests.cpp)
- [ ] T312 [P] [US8] 9 スライス / タイル描画 / マスクのテスト (tests/unit/render/SpriteDrawModeTests.cpp)
- [ ] T313 [P] [US8] SceneManager（単独/追加/非同期/解放/永続オブジェクト）のテスト (tests/integration/SceneManagerTests.cpp)
- [ ] T314 [P] [US8] 遷移演出（フェード）のテスト (tests/unit/scene/SceneTransitionTests.cpp)

### Implementation
- [ ] T315 [US8] Canvas / CanvasScaler (engine/include/sprout/ui/Canvas.hpp, engine/src/ui/Canvas.cpp)
- [ ] T316 [US8] UiElement 基底と RectLayout (engine/include/sprout/ui/UiElement.hpp, engine/src/ui/RectLayout.cpp)
- [ ] T317 [US8] UiEventSystem（ヒットテスト・ホバー・クリック・フォーカス）(engine/src/ui/UiEventSystem.cpp)
- [ ] T318 [P] [US8] Panel / Image / Text ウィジェット (engine/include/sprout/ui/Image.hpp, engine/include/sprout/ui/Text.hpp, engine/src/ui/widgets/BasicVisuals.cpp)
- [ ] T319 [P] [US8] Button / Toggle / Slider / ProgressBar (engine/include/sprout/ui/Button.hpp, engine/include/sprout/ui/Toggle.hpp, engine/include/sprout/ui/Slider.hpp, engine/include/sprout/ui/ProgressBar.hpp, engine/src/ui/widgets/BasicControls.cpp)
- [ ] T320 [P] [US8] InputField / Dropdown / ScrollView / Mask (engine/include/sprout/ui/InputField.hpp, engine/include/sprout/ui/Dropdown.hpp, engine/include/sprout/ui/ScrollView.hpp, engine/src/ui/widgets/AdvancedControls.cpp)
- [ ] T321 [P] [US8] LayoutGroup / ContentSizeFitter (engine/include/sprout/ui/LayoutGroup.hpp, engine/src/ui/LayoutGroup.cpp)
- [ ] T322 [US8] UiNavigation (engine/src/ui/UiNavigation.cpp)
- [ ] T323 [P] [US8] SpriteRenderer に Sliced / Tiled / Mask を追加 (engine/src/render/SpriteRenderer.cpp, engine/src/render/SpriteMask.cpp)
- [ ] T324 [P] [US8] UI のトゥイーン/アニメーション対応 (engine/src/ui/UiAnimationBinding.cpp)
- [ ] T325 [US8] 公開 SceneManager（単独/追加/非同期/永続化）(engine/include/sprout/scene/SceneManager.hpp, engine/src/scene/SceneManagerImpl.cpp)
- [ ] T326 [P] [US8] SceneTransition（フェード・読込中表示）(engine/src/scene/SceneTransition.cpp)
- [ ] T327 [P] [US8] UI 矩形ギズモ・アンカープリセット・キャンバスプレビュー (editor/app/src/gizmo/UiRectGizmo.cpp, editor/app/src/inspector/AnchorPresetDrawer.cpp)
- [ ] T328 [P] [US8] UI コンポーネント用インスペクタ (editor/app/src/inspector/UiInspectors.cpp)
- [ ] T329 [P] [US8] E2E: ボタン配置→イベント接続→再生でクリック (tests/editor_e2e/UiWorkflowTests.cpp)
- [ ] T330 [US8] サンプル 02_Breakout (samples/02_Breakout/)
- [ ] T331 [P] [US8] チュートリアル 9 章「UI と画面遷移」(docs/tutorial/09-ui-and-scenes.md)

---

## Phase 11: User Story 9 - 音と演出 (P3)

### Tests
- [ ] T332 [P] [US9] VoicePool（同時発音上限・優先度で停止）のテスト (tests/unit/audio/VoicePoolTests.cpp)
- [ ] T333 [P] [US9] ミキサーグループ木（乗算音量・ミュート）のテスト (tests/unit/audio/MixerGroupTests.cpp)
- [ ] T334 [P] [US9] 空間化（距離減衰・定位）のテスト (tests/unit/audio/SpatialAudioTests.cpp)
- [ ] T335 [P] [US9] BGM クロスフェード/フェードのテスト (tests/unit/audio/MusicPlayerTests.cpp)
- [ ] T336 [P] [US9] パーティクルモジュール評価（発生・形状・寿命・時間変化）のテスト (tests/unit/render/ParticleModuleTests.cpp)
- [ ] T337 [P] [US9] パーティクルシミュレータ（ワールド/ローカル空間・ジョブ並列結果一致）のテスト (tests/unit/render/ParticleSimulatorTests.cpp)
- [ ] T338 [P] [US9] カメラ追従（遅延・先読み・不感帯・範囲制限・揺れ・ピクセルパーフェクト）のテスト (tests/unit/render/CameraFollowTests.cpp)
- [ ] T339 [P] [US9] マテリアル/シェーダパラメータのテスト (tests/unit/render/MaterialTests.cpp)
- [ ] T340 [P] [US9] ライトマップ合成とポストエフェクト連鎖のテスト (tests/unit/render/LightingPostFxTests.cpp)
- [ ] T341 [P] [US9] リッチテキスト解析（色/太字/サイズ・縁取り・影）のテスト (tests/unit/render/RichTextTests.cpp)

### Implementation
- [ ] T342 [US9] AudioSystem / VoicePool (engine/src/audio/AudioSystem.cpp, engine/src/audio/VoicePool.cpp)
- [ ] T343 [US9] MixerGroup 木 (engine/src/audio/MixerGraph.cpp)
- [ ] T344 [P] [US9] 公開 AudioSource / AudioListener（空間化）(engine/include/sprout/audio/AudioSource.hpp, engine/include/sprout/audio/AudioListener.hpp, engine/src/audio/AudioSource.cpp)
- [ ] T345 [P] [US9] MusicPlayer（ストリーム・クロスフェード）と公開 Audio (engine/src/audio/MusicPlayer.cpp, engine/include/sprout/audio/Audio.hpp)
- [ ] T346 [US9] ParticleEffectData とインポーター (engine/src/render/particles/ParticleEffectData.cpp)
- [ ] T347 [US9] ParticleSimulator（JobSystem）(engine/src/render/particles/ParticleSimulator.cpp)
- [ ] T348 [US9] ParticleRenderer（バッチ）と公開 ParticleSystem (engine/src/render/particles/ParticleRenderer.cpp, engine/include/sprout/render/ParticleSystem.hpp)
- [ ] T349 [P] [US9] 公開 CameraFollow / CameraShake / PixelPerfect (engine/include/sprout/render/CameraFollow.hpp, engine/src/render/CameraEffects.cpp)
- [ ] T350 [P] [US9] Material / Shader アセットと公開 Material (engine/src/render/MaterialData.cpp, engine/include/sprout/render/Material.hpp)
- [ ] T351 [P] [US9] Light2D とライトマップ・影キャスター (engine/include/sprout/render/Light2D.hpp, engine/src/render/lighting/LightMapRenderer.cpp, engine/src/render/lighting/ShadowCaster.cpp)
- [ ] T352 [P] [US9] PostEffectStack（ブルーム・ビネット・色調補正・ピクセル化）(engine/include/sprout/render/PostEffectStack.hpp, engine/src/render/postfx/*.cpp, engine/resources/shaders/postfx/*.frag)
- [ ] T353 [P] [US9] RichText / 縁取り / 影 (engine/src/render/text/RichTextParser.cpp, engine/src/render/text/TextEffects.cpp)
- [ ] T354 [P] [US9] パーティクルエディタ（プレビュー・モジュール編集）(editor/app/src/window/ParticleEditorWindow.cpp)
- [ ] T355 [P] [US9] オーディオミキサー設定 UI (editor/app/src/window/AudioMixerPanel.cpp)
- [ ] T356 [P] [US9] マテリアル / ライト / ポスト FX インスペクタ (editor/app/src/inspector/RenderingInspectors.cpp)
- [ ] T357 [P] [US9] 性能: パーティクル 20,000 個 60 fps (tests/perf/ParticleStressTests.cpp)

---

## Phase 12: User Story 10 - 入力設定 (P3)

### Tests
- [ ] T358 [P] [US10] Gamepad（XInput + DB 正規化・最大 4 台・接続/切断イベント）のテスト (tests/unit/input/GamepadTests.cpp)
- [ ] T359 [P] [US10] プロセッサ（不感帯・感度・反転・正規化）のテスト (tests/unit/input/InputProcessorTests.cpp)
- [ ] T360 [P] [US10] リバインド（衝突検出・保存・復元）のテスト (tests/unit/input/RebindTests.cpp)
- [ ] T361 [P] [US10] テキスト入力・IME のテスト (tests/unit/input/TextInputTests.cpp)

### Implementation
- [ ] T362 [US10] GamepadDevice（XInput・SFML Joystick・マッピング DB）(engine/src/input/GamepadDevice.cpp, engine/resources/gamecontrollerdb.txt)
- [ ] T363 [P] [US10] InputProcessor 群 (engine/src/input/InputProcessors.cpp)
- [ ] T364 [US10] RebindService と BindingOverride 永続化 (engine/src/input/RebindService.cpp)
- [ ] T365 [P] [US10] TextInput / IME (engine/src/input/TextInput.cpp)
- [ ] T366 [P] [US10] 公開 GamepadButton とデバイス変更イベント (engine/include/sprout/input/GamepadButton.hpp)
- [ ] T367 [P] [US10] InputActions エディタ（マップ/アクション/割当/合成・キー入力で割当）(editor/app/src/window/InputActionsWindow.cpp)
- [ ] T368 [P] [US10] E2E: アクション追加→割当→再生で反応 (tests/editor_e2e/InputActionsWorkflowTests.cpp)

---

## Phase 13: User Story 11 - セーブ・多言語化 (P3)

### Tests
- [ ] T369 [P] [US11] SaveData 木（型・入れ子・往復）のテスト (tests/unit/persistence/SaveDataTests.cpp)
- [ ] T370 [P] [US11] SaveSystem（スロット・バックアップ・チェックサム破損検出→復旧・版移行）のテスト (tests/unit/persistence/SaveSystemTests.cpp)
- [ ] T371 [P] [US11] PlayerPrefs のテスト (tests/unit/persistence/PlayerPrefsTests.cpp)
- [ ] T372 [P] [US11] StringTable と Localization（切替・欠落キー・ロケール別フォント）のテスト (tests/unit/persistence/LocalizationTests.cpp)

### Implementation
- [ ] T373 [US11] SaveData / SaveSystem (engine/include/sprout/persistence/SaveSystem.hpp, engine/src/persistence/SaveSystem.cpp)
- [ ] T374 [P] [US11] PlayerPrefs (engine/include/sprout/persistence/PlayerPrefs.hpp, engine/src/persistence/PlayerPrefs.cpp)
- [ ] T375 [US11] StringTable アセット・Localization (engine/src/persistence/StringTableData.cpp, engine/include/sprout/persistence/Localization.hpp, engine/src/persistence/Localization.cpp)
- [ ] T376 [P] [US11] UI Text の文字列キー結合 (engine/src/ui/LocalizedTextBinding.cpp)
- [ ] T377 [P] [US11] 文字列テーブルエディタ（言語列・欠落表示・CSV 入出力）(editor/app/src/window/LocalizationWindow.cpp)
- [ ] T378 [P] [US11] E2E: 言語切替で UI テキスト即時変更 (tests/editor_e2e/LocalizationWorkflowTests.cpp)

---

## Phase 14: User Story 12 - 書き出し (P3)

### Tests
- [ ] T379 [P] [US12] BuildProfile 往復のテスト (tests/unit/build/BuildProfileTests.cpp)
- [ ] T380 [P] [US12] 使用アセット収集（未使用除外）のテスト (tests/unit/build/UsedAssetCollectorTests.cpp)
- [ ] T381 [P] [US12] .spak 書込/読込/マウント・改ざん検出のテスト (tests/unit/build/PackTests.cpp)
- [ ] T382 [P] [US12] 書き出し→プレイヤーをヘッドレス起動の統合テスト (tests/integration/BuildExportTests.cpp)

### Implementation
- [ ] T383 [US12] BuildProfile (editor/core/src/build/BuildProfile.cpp)
- [ ] T384 [US12] UsedAssetCollector (editor/core/src/build/UsedAssetCollector.cpp)
- [ ] T385 [US12] PackWriter / PackMount (engine/src/asset/pack/PackWriter.cpp, engine/src/core/fs/PackMount.cpp)
- [ ] T386 [US12] ExportPipeline（モジュール Release ビルド→パック→配置→アイコン/版情報）(editor/core/src/build/ExportPipeline.cpp)
- [ ] T387 [US12] SproutPlayer（起動設定・パックマウント・モジュール読込）(player/src/PlayerMain.cpp, player/CMakeLists.txt)
- [ ] T388 [P] [US12] SPROUT_MONOLITHIC 静的結合構成 (cmake/SproutMonolithic.cmake)
- [ ] T389 [P] [US12] 開発ビルド/製品ビルドの機能除外（DebugDraw・コンソール・プロファイラ）(engine/src/core/BuildFlags.hpp)
- [ ] T390 [P] [US12] プレイヤーの致命的エラーダイアログとクラッシュログ (player/src/FatalErrorDialog.cpp)
- [ ] T391 [P] [US12] BuildSettings ウィンドウ（シーン順・プロファイル・書き出し進捗）(editor/app/src/window/BuildSettingsWindow.cpp)
- [ ] T392 [P] [US12] クリーン PC 検証手順書 (docs/release/clean-machine-check.md)

---

## Phase 15: User Story 13 - 診断 (P3)

### Tests
- [ ] T393 [P] [US13] Profiler（マーカー入れ子・リングバッファ・統計）のテスト (tests/unit/diagnostics/ProfilerTests.cpp)
- [ ] T394 [P] [US13] DebugDraw（持続時間・製品ビルドで無効）のテスト (tests/unit/diagnostics/DebugDrawTests.cpp)
- [ ] T395 [P] [US13] ConsoleCommand（登録・引数解析・補完）のテスト (tests/unit/diagnostics/ConsoleCommandTests.cpp)

### Implementation
- [ ] T396 [US13] Profiler と公開計測マーカー (engine/src/diagnostics/Profiler.cpp, engine/include/sprout/diagnostics/Profiler.hpp)
- [ ] T397 [P] [US13] Tracy ブリッジ（任意）(engine/src/diagnostics/TracyBridge.cpp)
- [ ] T398 [P] [US13] 公開 DebugDraw (engine/include/sprout/diagnostics/DebugDraw.hpp, engine/src/diagnostics/DebugDraw.cpp)
- [ ] T399 [P] [US13] 可視化切替（物理・カメラ範囲・UI 境界）(engine/src/diagnostics/DebugViews.cpp)
- [ ] T400 [P] [US13] ConsoleCommand と開発用コンソール UI (engine/include/sprout/diagnostics/ConsoleCommand.hpp, engine/src/diagnostics/DevConsole.cpp)
- [ ] T401 [P] [US13] メモリ・オブジェクト数・アセット数統計 (engine/src/diagnostics/RuntimeStats.cpp)
- [ ] T402 [P] [US13] Profiler ウィンドウ（時系列グラフ・システム別・描画呼出し）(editor/app/src/window/ProfilerWindow.cpp)
- [ ] T403 [P] [US13] チュートリアル 10 章「デバッグと最適化」(docs/tutorial/10-debugging.md)

---

## Phase 16: User Story 14 - 教員の配布・回収 (P4)

### Tests
- [ ] T404 [P] [US14] CLI 引数解析と終了コードのテスト (tests/unit/cli/CliParserTests.cpp)
- [ ] T405 [P] [US14] new / build / export / test / validate / template register の統合テスト (tests/integration/CliCommandTests.cpp)

### Implementation
- [ ] T406 [US14] sprout_cli 本体と引数解析 (cli/src/CliMain.cpp, cli/src/CliParser.cpp)
- [ ] T407 [P] [US14] new / template register (cli/src/commands/NewCommand.cpp, cli/src/commands/TemplateCommand.cpp)
- [ ] T408 [P] [US14] build / export (cli/src/commands/BuildCommand.cpp, cli/src/commands/ExportCommand.cpp)
- [ ] T409 [P] [US14] test / validate（Missing 参照・GUID 重複・版検査）(cli/src/commands/TestCommand.cpp, cli/src/commands/ValidateCommand.cpp)
- [ ] T410 [P] [US14] テンプレート 2DPlatformer / 2DShooter (templates/2DPlatformer/, templates/2DShooter/)
- [ ] T411 [P] [US14] 教員向けガイド（課題配布・一括ビルド・採点補助）(docs/instructor/guide.md)

---

## Phase 17: User Story 15 - エディタ拡張 (P4)

### Tests
- [ ] T412 [P] [US15] SerializedObject / SerializedProperty（Undo・Prefab 上書き・dirty 連動）のテスト (tests/unit/editor/SerializedObjectTests.cpp)
- [ ] T413 [P] [US15] 拡張登録（CustomInspector / PropertyDrawer / EditorTool / AssetImporter・モジュール再読込で再登録）のテスト (tests/unit/editor/ExtensionRegistryTests.cpp)

### Implementation
- [ ] T414 [US15] SerializedObject / SerializedProperty (editor/core/src/extension/SerializedObject.cpp, engine/include/sprout/editor/SerializedObject.hpp)
- [ ] T415 [US15] EditorGui ラッパ (engine/include/sprout/editor/EditorGui.hpp, editor/app/src/extension/EditorGui.cpp)
- [ ] T416 [US15] ExtensionRegistry とマクロ SPROUT_CUSTOM_INSPECTOR / SPROUT_EDITOR_TOOL (engine/include/sprout/editor/Extension.hpp, editor/core/src/extension/ExtensionRegistry.cpp)
- [ ] T417 [P] [US15] 独自 AssetImporter 登録 (engine/include/sprout/editor/AssetImporter.hpp, editor/core/src/extension/CustomImporterBridge.cpp)
- [ ] T418 [P] [US15] E2E: カスタムインスペクタ表示 (tests/editor_e2e/ExtensionTests.cpp)
- [ ] T419 [US15] サンプル 05_Puzzle（レベル生成ツール拡張つき）(samples/05_Puzzle/)
- [ ] T420 [P] [US15] 拡張ガイド (docs/guides/editor-extension.md)

---

## Phase 18: Polish & Release（横断）

- [ ] T421 サンプル 04_Shooter（UI・遷移・音・入力設定・セーブ・多言語を統合）(samples/04_Shooter/)
- [ ] T422 [P] アーキテクチャ解説（モジュール図・フレーム順序・ER 図への導線）(docs/architecture/overview.md)
- [ ] T423 [P] Doxygen 公開 API 網羅率 100% 検査を CI に追加 (.github/workflows/docs.yml)
- [ ] T424 [P] カバレッジ計測とコア 80% ゲート (.github/workflows/ci.yml, cmake/SproutCoverage.cmake)
- [ ] T425 [P] 全テストの ASan 実行を CI 必須化 (.github/workflows/ci.yml)
- [ ] T426 [P] 8 時間耐久テスト自動化（再生/停止 500 回・再読込 100 回・メモリ監視）(tests/soak/EditorSoakTests.cpp)
- [ ] T427 [P] 性能回帰の基準値記録と CI 比較 (tests/perf/baseline.json, .github/workflows/perf.yml)
- [ ] T428 [P] セキュリティ: パック/セーブ/シーン読込の入力検証・パストラバーサル防止のファジング (tests/fuzz/FileFormatFuzzTests.cpp)
- [ ] T429 [P] 高 DPI・複数モニタ・日本語パスの手動検証表 (docs/release/manual-test-matrix.md)
- [ ] T430 [P] エディタ英語 UI の網羅 (editor/app/resources/strings.en.json)
- [ ] T431 [P] 第三者ライセンス表記の最終確認 (THIRD_PARTY_NOTICES.md)
- [ ] T432 [P] CHANGELOG とリリースノート (CHANGELOG.md)
- [ ] T433 [P] 配布パッケージ（エディタ + CLI + テンプレート + サンプル zip）作成スクリプト (tools/package_release.ps1)
- [ ] T434 ER 図とコードの最終整合レビュー (specs/001-student-2d-engine/er-diagram.md)
- [ ] T435 quickstart 全 12 シナリオを VS2022 / VS2026 両方で実施し記録 (specs/001-student-2d-engine/quickstart.md)
- [ ] T436 README 更新（機能一覧・導入手順・サンプル紹介）(README.md)

---

## Dependencies & Execution Order

### フェーズ依存
- Setup → Foundational（M0）→ **US1** → US2 → US3（ここまで P1 = MVP）
- US4 / US5 / US6 / US8 は US3 完了後に**並行可**。US7 は US5（タイル衝突）完了後。
- US9 / US10 / US11 / US13 は US8 完了後に並行可。US12 は US3 + US11 完了後。
- US14 は US12 後、US15 は US2 + US3 後いつでも可。Polish は全 US 後。

### ストーリー内
- テスト（失敗確認）→ 内部実装（detail）→ 公開 API → エディタ UI → E2E → ドキュメント

### Parallel Example: Foundational 2-1
```text
T021〜T034（数学: テストと実装の各組）を 7 組並行
```

## Implementation Strategy
1. **MVP（M0 + US1）**: 授業 1〜2 回目で試用し API の使い勝手を確認（約 5 週）
2. **エディタ MVP（+US2, US3）**: 書く→見るサイクル完成（+5 週）
3. **制作機能（US4〜US8）**: 4 本柱を並行、各完了ごとにサンプル更新（+8 週）
4. **仕上げ（US9〜US15, Polish）**（+9 週）
