# Implementation Plan: 学生向け 2D ゲームエンジン + エディタ（Sprout Engine）

**Branch**: `001-student-2d-engine` | **Date**: 2026-10-21 | **Spec**: [spec.md](spec.md)

**Input**: Feature specification from `/specs/001-student-2d-engine/spec.md`

## Summary

SFML 3 + C++20 の 2D エンジン `sprout`（静的ライブラリ）と、ImGui-SFML 製エディタ `SproutEditor` を作る。利用者は `include/sprout/` の Facade API（`Game` / `Scene` / `GameObject` / `Behavior` / 各 Component）だけを使い、内部は `sprout::detail` に隔離する。シーンは JSON で保存し、エディタは Command パターンで Undo/Redo、再生時はシーンを複製して実行・停止時に破棄する。

## Technical Context

**Language/Version**: C++20（MSVC v143 = VS2022 / v145 = VS2026）
**Primary Dependencies**: SFML 3.0（graphics, window, audio, system）, imgui + imgui-sfml, nlohmann-json, GoogleTest（すべて vcpkg マニフェスト）
**Storage**: ファイル（シーン `.scene.json`、プロジェクト `sprout.project.json`、アセットは画像/音声/フォント）
**Testing**: GoogleTest + CTest。ロジックはヘッドレス（ウィンドウ生成なし）でテスト
**Target Platform**: Windows 10/11 x64（Linux は努力目標）
**Project Type**: desktop-app（ライブラリ + エディタ + サンプル）
**Performance Goals**: 1,000 スプライトで 60 fps、エディタ操作反映 100 ms 以内
**Constraints**: 公開 API は例外を投げない、公開ヘッダに SFML/ImGui/json 型を露出しない、`/W4 /permissive- /WX` で警告ゼロ
**Scale/Scope**: 公開型 約 20、エディタパネル 5、サンプル 2

## Constitution Check

| 原則 | ゲート | 判定 |
|---|---|---|
| I. Facade First | 公開ヘッダは `include/sprout/` のみ、外部ライブラリ型を露出しない（PImpl / ハンドル） | PASS（contracts/public-api.md で定義） |
| II. 命名規則 | `.clang-tidy` readability-identifier-naming で検査、CI ターゲット `sprout_lint` | PASS（T004/T005） |
| III. 責務分割 | モジュール分割と依存方向 `editor → sprout → sprout::detail → SFML` | PASS（下記 Structure） |
| IV. テスト必須 | 各ストーリーでテスト先行タスクを配置 | PASS（tasks.md） |
| V. ビルド再現性 | `CMakePresets.json` に `vs2022` / `vs2026` | PASS（T002） |

Post-design 再評価: 違反なし。Complexity Tracking 不要。

## Project Structure

### Documentation (this feature)

```text
specs/001-student-2d-engine/
├── plan.md
├── research.md
├── data-model.md
├── quickstart.md
├── contracts/
│   ├── public-api.md
│   └── scene-file-format.md
└── tasks.md
```

### Source Code (repository root)

```text
CMakeLists.txt
CMakePresets.json            # vs2022 / vs2026 × debug / release
vcpkg.json
.clang-format
.clang-tidy

engine/                      # ライブラリ sprout
├── include/sprout/          # 公開ヘッダ（利用者が見るのはここだけ）
│   ├── Sprout.hpp           # 一括インクルード
│   ├── core/       Game.hpp, GameConfig.hpp, Time.hpp, Log.hpp, Result.hpp
│   ├── math/       Vector2.hpp, Color.hpp, Rect.hpp
│   ├── scene/      Scene.hpp, GameObject.hpp
│   ├── component/  Component.hpp, Transform.hpp, SpriteRenderer.hpp, TextRenderer.hpp,
│   │               BoxCollider.hpp, TileMap.hpp, AudioSource.hpp, Behavior.hpp
│   ├── input/      Input.hpp, Key.hpp, MouseButton.hpp
│   ├── audio/      Audio.hpp
│   └── reflect/    Property.hpp, BehaviorRegistry.hpp   # SPROUT_REGISTER_BEHAVIOR
└── src/                     # 非公開（sprout::detail）
    ├── core/       GameLoop, Window, LogSink
    ├── scene/      SceneImpl, ObjectRegistry, ObjectHandle
    ├── render/     Renderer, SpriteBatch, Camera
    ├── input/      InputState
    ├── physics/    CollisionWorld, Aabb
    ├── asset/      AssetCache<T>, TextureLoader, FallbackAssets
    ├── audio/      AudioMixer
    └── serialization/ SceneSerializer, SceneDeserializer, ComponentCodec

editor/                      # 実行ファイル SproutEditor（namespace sprout::editor）
└── src/
    ├── EditorApp.*  EditorContext.*  Selection.*
    ├── panel/      Panel.hpp, SceneTreePanel, InspectorPanel, ViewportPanel, AssetBrowserPanel, LogPanel
    ├── command/    EditorCommand.hpp, CommandHistory, CreateObjectCommand, DeleteObjectCommand,
    │               SetPropertyCommand, ReparentCommand, MoveObjectCommand
    ├── gizmo/      TranslateGizmo
    └── play/       PlayModeController

samples/  HelloSprout/, TileCollector/
templates/StudentProject/    # 学生配布用テンプレート
tests/
├── unit/          # 型ごとの <Type>Tests.cpp
└── integration/   # シーン往復, エディタコマンド, プレイモード
```

**Structure Decision**: エンジンを静的ライブラリ `sprout`、エディタを `SproutEditor` 実行ファイルとして分離。CMake で `sprout` の `PUBLIC` include は `engine/include` のみ、`engine/src` は `PRIVATE`。これにより利用者は内部ヘッダを include できない（FR-018）。エディタは公開 API + `sprout::editor_access`（エディタ専用の限定インタフェース、`engine/include/sprout/editor_access/` を `SproutEditor` にのみ公開）を使う。

## Complexity Tracking

違反なし。
