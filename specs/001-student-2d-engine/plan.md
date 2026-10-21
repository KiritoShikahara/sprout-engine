# Implementation Plan: Sprout Engine — 商業水準 2D エンジン + 統合エディタ

**Branch**: `001-student-2d-engine` | **Date**: 2026-10-21 | **Spec**: [spec.md](spec.md)（v2） | **ER**: [er-diagram.md](er-diagram.md)

**Input**: Feature specification from `/specs/001-student-2d-engine/spec.md`

## Summary

SFML 3 + C++20 で、ランタイム（`SproutEngine.dll`）・エディタ（`SproutEditor.exe`）・プレイヤー（`SproutPlayer.exe`）・学生ゲームモジュール（`<Game>.dll`）の 4 成果物からなる 2D エンジンを作る。

- **利用者境界**: 学生が触るのは `engine/include/sprout/` の公開ヘッダのみ。実体はハンドル（index + generation）で参照し、内部テーブル（ER 図 2）は `sprout::detail` に隔離する。
- **ゲームコード**: 学生コードは常に DLL としてビルドし、エディタはシャドウコピーして読み込む。再読み込みはプレイモードと同じ「ワールドのシリアライズ → 型の差し替え → 復元」で実現する（同一機構の再利用でテストを一本化）。
- **基盤ライブラリ**: 描画/窓/音 = SFML 3、物理 = Box2D v3、UI = Dear ImGui (docking) + ImGui-SFML、保存 = JSON（開発）/ パック（書き出し）。
- **品質**: GoogleTest + ImGui Test Engine、clang-tidy による命名規則検査、GitHub Actions（VS2022）+ 自前ランナー（VS2026）。

## Technical Context

**Language/Version**: C++20（MSVC v143 = VS2022 17.10+、v145 = VS2026）。エンジン・エディタ・ゲームモジュールは同一ツールセット系列でビルド（v14x は ABI 互換）
**Primary Dependencies**（vcpkg マニフェスト、バージョン固定 `builtin-baseline`）:
SFML 3.0 / box2d 3.x / imgui[docking-experimental] / imgui-sfml / imnodes / nlohmann-json / zstd / efsw / nativefiledialog-extended / tracy（任意）/ gtest / imgui-test-engine（教育・非商用ライセンスで使用）
**Storage**: プロジェクトフォルダ（`Assets/`, `ProjectSettings/`, `Library/`=キャッシュ, `Build/`）。テキスト JSON + `.meta`。書き出し時は `.spak`（zstd 圧縮パック）
**Testing**: GoogleTest（unit / integration / determinism / serialization round-trip）、ImGui Test Engine（エディタ E2E）、性能ベンチ（tests/perf）、CTest でラベル分け
**Target Platform**: Windows 10/11 x64（Linux は CI でランタイムのみ努力目標）
**Project Type**: desktop-app（ランタイム DLL + エディタ + プレイヤー + テンプレート + サンプル）
**Performance Goals**: SC-005（10k スプライト / 1k 剛体 / 20k パーティクルで 60 fps）、SC-004（変更→再生 10 秒以内）、SC-006（エディタ反映 100 ms、1k オブジェクト 2 秒でロード）
**Constraints**: 公開 API は例外を投げない・外部ライブラリ型を露出しない・`/W4 /permissive- /WX /utf-8`・利用者コードはメインスレッドのみ
**Scale/Scope**: 公開型 約 120、エディタウィンドウ 約 20、FR 119、サンプル 5、チュートリアル 10 章、想定コード量 8〜12 万行

## Constitution Check

*GATE: Phase 0 前と Phase 1 後に評価*

| 原則 | ゲート | 判定（前） | 判定（後） |
|---|---|---|---|
| I. Facade First | 公開ヘッダに SFML/Box2D/ImGui/json 型が無い。組込みコンポーネントは公開クラス + `friend detail::ComponentAccess`、データは `m_` 非公開 | PASS | PASS（contracts/public-api.md、CI で `PublicHeaderIsolationTests` が include グラフを検査） |
| I. グローバル非公開 | v1 の `Input::` 静的関数を廃止し、`Behavior` 経由のコンテキスト（`input()`, `time()`, `audio()`, `scene()`, `assets()`）に変更 | PASS | PASS |
| II. 命名規則 | `.clang-tidy` readability-identifier-naming、`sprout_lint` を CI 必須 | PASS | PASS |
| III. 責務分割・依存方向 | モジュール DAG（下図）を CMake ターゲットで強制。逆依存はリンクエラーになる | PASS | PASS |
| IV. テスト必須 | 全モジュールにテストターゲット。決定性・往復・Undo・Prefab 上書きの専用スイート | PASS | PASS |
| V. ビルド再現性 | `CMakePresets.json`（vs2022 / vs2026 / ninja-msvc / ci）、vcpkg baseline 固定 | PASS | PASS |

**例外（Complexity Tracking へ記録）**: 公開マクロ `SPROUT_BEHAVIOR` / `SPROUT_PROPERTY` / `SPROUT_GAME_MODULE` の 3 つ（constitution II は 1 つのみ許可 → 改訂提案）。

## Architecture

### モジュール依存（上が下に依存。逆方向・横方向の未記載依存は禁止）

```text
SproutEditor.exe ─┐                       SproutPlayer.exe ─┐
                  ▼                                          ▼
           sprout_editor_core ──────────────► SproutEngine.dll（公開 API = include/sprout）
                  │                                          │
<Game>.dll ───────┴────── links ───────────► SproutEngine.dll
                                                             │
  SproutEngine.dll の内部（静的ライブラリを 1 つの DLL に結合）:
  facade ─► scene ─► reflection ─► serialization
     │        │
     ├► render ─► asset ─► platform ─► core
     ├► physics(Box2D) ─────────────────► core
     ├► animation ─► scene
     ├► audio ─► asset
     ├► input ─► platform
     ├► ui ─► render, input
     └► diagnostics ─► core
```

| CMake ターゲット | 種別 | 名前空間 | 責務 |
|---|---|---|---|
| `sprout_core` | static | `sprout::detail` | 数学, Result, ログ, イベントバス, ジョブ, 時間, メモリアロケータ, GUID, ファイルシステム抽象 |
| `sprout_platform` | static | `sprout::detail` | SFML ウィンドウ/イベント, 高 DPI, クリップボード, クラッシュハンドラ（SEH/minidump） |
| `sprout_reflection` | static | `sprout::detail` | TypeRegistry, TypeInfo, FieldInfo, 属性 |
| `sprout_serialization` | static | `sprout::detail` | JSON/パック読み書き, 版移行（Migrator 連鎖）, fileId ↔ handle 対応 |
| `sprout_asset` | static | `sprout::detail` | AssetDatabase, Importer 群, AssetManager（参照カウント・非同期）, ホットリロード, パック |
| `sprout_scene` | static | `sprout::detail` | World, ObjectTable, ComponentPool, PendingQueue, Hierarchy, Prefab 展開と上書き |
| `sprout_render` | static | `sprout::detail` | RenderSystem, ソートキー, バッチャ, カメラ, マテリアル/シェーダ, ライト 2D, ポスト FX, パーティクル, テキスト |
| `sprout_physics` | static | `sprout::detail` | Box2D ラッパ, 固定ステップ, 補間, 接触イベント, クエリ, ジョイント |
| `sprout_animation` | static | `sprout::detail` | クリップ再生, アニメーター状態機械, トゥイーン |
| `sprout_audio` | static | `sprout::detail` | ボイス管理, ミキサーグループ, 空間化, クロスフェード |
| `sprout_input` | static | `sprout::detail` | デバイス状態, アクションマップ, 合成/プロセッサ, リバインド, ゲームパッド DB |
| `sprout_ui` | static | `sprout::detail` | キャンバス, レイアウト, ウィジェット, ナビゲーション |
| `sprout_diagnostics` | static | `sprout::detail` | プロファイラ, デバッグ描画, コンソールコマンド |
| `SproutEngine` | **shared** | `sprout` | 公開ファサード。上記を結合し `SPROUT_API` で公開 API のみ輸出 |
| `sprout_editor_core` | static | `sprout::editor` | コマンド/Undo, 選択, ドキュメント, プレイモード, モジュールホスト, ビルドサービス（ImGui 非依存＝テスト容易） |
| `SproutEditor` | exe | `sprout::editor` | ImGui 層: ウィンドウ, ギズモ, 専用エディタ群 |
| `SproutPlayer` | exe | — | 書き出し版起動。`SPROUT_MONOLITHIC=ON` でゲームとエンジンを 1 exe に静的結合（FR-155） |
| `sprout_cli` | exe | `sprout::cli` | `sprout build / export / test / new`（FR-602, US14） |

### 主要設計判断（詳細は research.md）
1. **ハンドル + プール**: `GameObject` / `Ref<T>` は 16 byte 値型。`operator->` 毎回解決（デバッグで世代検査、リリースでも検査、失敗時ログ + ダミー参照を返し UB を防ぐ）
2. **型登録**: `SPROUT_BEHAVIOR(PlayerMover)` が静的登録子を生成し、モジュール読込時に `TypeRegistry` へ一括登録。プロパティは `SPROUT_PROPERTY(m_speed, Range(0, 500))` もしくは `declareProperties()`
3. **フレーム順序**: Input → EarlyUpdate(Pending 反映) → FixedUpdate×n（物理ステップ・接触通知）→ Update → Animation → LateUpdate → Coroutines/Timers → Transform 確定 → Audio → Render 収集/ソート/バッチ → UI → DebugDraw → Present → 破棄反映
4. **エディタ再読込/プレイ**: `WorldSnapshot`（シリアライズ済みバイト列）を共通部品化。プレイ開始/停止・モジュール再読込・自動保存・Undo の大規模操作が同じ経路を通る
5. **クラッシュ保護**: 利用者コード呼出しは `detail::UserCodeGuard`（SEH `__try` + `std::exception` 捕捉）で囲み、エディタ時は再生停止 + 復元、プレイヤー時は minidump + エラーダイアログ
6. **アセット参照**: 公開型 `AssetRef<Texture>` 等（GUID 保持・遅延解決）。パス文字列 API は簡便用に併設し内部で GUID へ解決

## Project Structure

### Documentation (this feature)

```text
specs/001-student-2d-engine/
├── spec.md / er-diagram.md
├── plan.md / research.md / data-model.md / quickstart.md
├── contracts/
│   ├── public-api.md          # 学生向け公開 API
│   ├── game-module-abi.md     # ゲーム DLL とエンジン/エディタの契約
│   ├── file-formats.md        # .meta / .scene / .prefab / 設定 / .spak
│   ├── editor-extension-api.md
│   └── cli.md                 # sprout_cli
├── checklists/requirements.md
└── tasks.md
```

### Source Code (repository root)

```text
CMakeLists.txt  CMakePresets.json  vcpkg.json  .clang-format  .clang-tidy
cmake/            SproutWarnings.cmake SproutLint.cmake SproutGameModule.cmake SproutVersion.cmake
engine/
├── include/sprout/                 # 公開ヘッダのみ（学生が見る）
│   ├── Sprout.hpp
│   ├── core/       Result.hpp Log.hpp Time.hpp Random.hpp Event.hpp Timer.hpp Coroutine.hpp
│   ├── math/       Vector2.hpp Matrix3.hpp Rect.hpp Circle.hpp Color.hpp Angle.hpp Easing.hpp MathUtil.hpp
│   ├── scene/      GameObject.hpp Ref.hpp Scene.hpp SceneManager.hpp Prefab.hpp
│   ├── component/  Component.hpp Behavior.hpp Transform.hpp
│   ├── render/     SpriteRenderer.hpp TextRenderer.hpp ShapeRenderer.hpp Camera.hpp CameraFollow.hpp Light2D.hpp
│   │               ParticleSystem.hpp PostEffectStack.hpp Material.hpp
│   ├── tilemap/    Tilemap.hpp
│   ├── physics/    Rigidbody2D.hpp Collider2D.hpp BoxCollider2D.hpp CircleCollider2D.hpp CapsuleCollider2D.hpp
│   │               PolygonCollider2D.hpp EdgeCollider2D.hpp Joint2D.hpp Physics2D.hpp Collision.hpp
│   ├── animation/  Animator.hpp Tween.hpp
│   ├── audio/      AudioSource.hpp AudioListener.hpp Audio.hpp
│   ├── input/      Input.hpp InputAction.hpp Key.hpp MouseButton.hpp GamepadButton.hpp
│   ├── ui/         Canvas.hpp UiElement.hpp Button.hpp Text.hpp Image.hpp Slider.hpp Toggle.hpp ProgressBar.hpp
│   │               InputField.hpp Dropdown.hpp ScrollView.hpp LayoutGroup.hpp
│   ├── asset/      AssetRef.hpp Texture.hpp Sprite.hpp Font.hpp AudioClip.hpp Assets.hpp
│   ├── persistence/ SaveSystem.hpp PlayerPrefs.hpp Localization.hpp
│   ├── diagnostics/ DebugDraw.hpp Profiler.hpp ConsoleCommand.hpp
│   ├── reflect/    Property.hpp Attributes.hpp Registration.hpp   # SPROUT_BEHAVIOR / SPROUT_PROPERTY
│   └── app/        Game.hpp GameConfig.hpp GameModule.hpp           # SPROUT_GAME_MODULE
├── include_editor/sprout/editor_access/   # エディタにのみ公開（学生テンプレートからは見えない）
└── src/<module>/...                       # 非公開。上表のモジュールごとにフォルダ
editor/
├── core/src/   command/ selection/ document/ playmode/ modulehost/ build/ project/ prefs/
└── app/src/    EditorApplication  window/{SceneView, GameView, Hierarchy, Inspector, AssetBrowser, Console,
                Profiler, SpriteEditor, AnimationEditor, AnimatorGraph, TilePalette, ParticleEditor,
                InputActionsEditor, LocalizationEditor, ProjectSettings, Preferences, BuildSettings, ProjectHub}
                gizmo/ inspector/(PropertyDrawer 群) theme/ fonts/(Noto Sans JP)
player/src/     PlayerMain.cpp
cli/src/        CliMain.cpp commands/
templates/      2DEmpty/ 2DPlatformer/ 2DShooter/   (project.json, Assets/, Game/CMakeLists.txt)
samples/        01_HelloSprout 02_Breakout 03_Platformer 04_Shooter 05_Puzzle
docs/           tutorial/(ja 10 章+) api/(Doxygen) architecture/(ER 図へのリンク・設計解説)
tests/
├── unit/<module>/        <Type>Tests.cpp
├── integration/          SceneRoundTrip, PrefabOverride, HotReload, PlayMode, BuildExport
├── determinism/          PhysicsDeterminism
├── editor_e2e/           ImGui Test Engine シナリオ
└── perf/                 SpriteStress, PhysicsStress, ParticleStress
.github/workflows/        ci.yml（vs2022 build+test+lint）, self-hosted-vs2026.yml, docs.yml
```

**Structure Decision**: ランタイムは共有ライブラリ（エディタ・プレイヤー・ゲームモジュールで単一インスタンスを共有するため必須）。内部は機能別の静的ライブラリに分割し、DAG を CMake の `target_link_libraries(PRIVATE)` で強制。エディタは ImGui 非依存の `sprout_editor_core` と ImGui 層 `SproutEditor` に分け、ロジックをヘッドレステスト可能にする。

## Phasing（tasks 生成の単位）

| マイルストーン | 含む US | 完了条件 | 目安 |
|---|---|---|---|
| M0 基盤 | — | ビルド/CI/lint、core, platform, reflection, serialization, scene, asset(最小) | 3 週 |
| M1 MVP ランタイム | US1 | 描画・入力アクション・時間・サンプル 01 | 2 週 |
| M2 エディタ基盤 | US2, US3 | プロジェクトハブ・主要 6 ウィンドウ・Undo・プレイ・ホットリロード | 5 週 |
| M3 ゲーム制作機能 | US4–US8 | アニメ・物理・Prefab・タイル・UI/遷移、サンプル 02–04 | 8 週 |
| M4 仕上げ機能 | US9–US13 | 音/演出・入力設定・セーブ/多言語・書き出し・診断 | 6 週 |
| M5 教育・拡張 | US14, US15 | CLI・テンプレート登録・拡張 API・チュートリアル・サンプル 05 | 3 週 |

合計 約 27 週（1 人フルタイム換算）。

## Complexity Tracking

| Violation | Why Needed | Simpler Alternative Rejected Because |
|---|---|---|
| 公開マクロ 3 種（constitution は 1 種） | C++20 に静的リフレクションが無く、型登録・プロパティ宣言・モジュール入口を 1 行で書かせるため | 手書き登録は学生の記述量が 3〜5 倍、コード生成ツールはビルド手順が増え 1 年生には重い |
| ランタイムを DLL 化 | エディタとゲームモジュールが同一エンジン状態を共有するため | 静的リンクでは DLL ごとにエンジン状態が複製され、ホットリロード不可 |
| imgui-test-engine の採用 | エディタ E2E テストの必須化（FR-902） | 手動テストは回帰検出不能 |
