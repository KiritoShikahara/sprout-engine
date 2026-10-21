# Sprout Engine Constitution

学生向け 2D ゲームエンジン「Sprout Engine」（SFML 3 + C++20 + エディタ）の開発規約。

## Core Principles

### I. 使う側の簡単さ最優先 (Facade First)
- 学生（エンジン利用者）が触るのは `include/sprout/` の公開ヘッダのみ。公開型は `sprout` 名前空間直下に置く。
- 内部実装は `sprout::detail` 名前空間と `src/` 内の非公開ヘッダに隔離し、公開ヘッダから SFML / ImGui / JSON ライブラリの型を露出しない（PImpl またはハンドルで隠蔽）。
- 公開 API は「1 つの目的に 1 つの呼び方」。オーバーロード乱立・出力引数・生ポインタ所有を禁止。
- 誤用はコンパイルエラーか、日本語で原因と対処を示すログで知らせる。未定義動作で落とさない。

### II. 命名規則 (NON-NEGOTIABLE)
コーディング開始前に本規約を確定済み。全コードはこれに従い、`.clang-format` / `.clang-tidy`（readability-identifier-naming）で機械的に検査する。

| 対象 | 規則 | 例 |
|---|---|---|
| 名前空間 | `snake_case`、短く | `sprout`, `sprout::detail`, `sprout::editor` |
| クラス / 構造体 / enum / concept / 型エイリアス | `PascalCase` | `GameObject`, `SpriteRenderer`, `ComponentType` |
| 関数 / メンバ関数 | `camelCase`、動詞始まり | `loadTexture()`, `findObject()` |
| bool を返す関数 | `is` / `has` / `can` 始まり | `isActive()`, `hasComponent<T>()` |
| ゲッター / セッター | `get` を付けない名詞 / `set` 始まり | `position()`, `setPosition()` |
| ローカル変数 / 引数 | `camelCase` | `deltaTime`, `texturePath` |
| 非 public メンバ変数 | `m_` + `camelCase` | `m_position` |
| static メンバ変数 | `s_` + `camelCase` | `s_instanceCount` |
| 定数 / `constexpr` | `k` + `PascalCase` | `kMaxObjects` |
| enum 値 | `PascalCase`（`k` は付けない） | `Key::Space` |
| テンプレート引数 | `PascalCase`、単一なら `T` | `TComponent` |
| マクロ | `SPROUT_` + `UPPER_SNAKE`、原則使用禁止 | `SPROUT_ASSERT`、公開は `SPROUT_REGISTER_BEHAVIOR` のみ |
| ファイル | 型名と同一 `PascalCase.hpp/.cpp`、1 ファイル 1 主要型 | `GameObject.hpp` |
| テスト | `<対象>Tests.cpp`、テスト名 `Method_Condition_Expected` | `Transform_SetParent_UpdatesWorldPosition` |

- enum は必ず `enum class`。略語は 1 語扱い（`JsonReader`, `loadUi`）。
- ヘッダは `.hpp`、`#pragma once` を使用。
- イベントハンドラ（利用者がオーバーライドする仮想関数）は `on` 始まり：`onStart()`, `onUpdate(float deltaTime)`。

### III. 責務分割とクラス設計
- 1 クラス 1 責務。モジュールは `core / scene / component / render / input / audio / physics / asset / serialization / editor` に分ける。
- 依存方向は `editor → engine(public) → engine(detail) → SFML`。逆方向依存とモジュール間循環を禁止。
- 所有権は `std::unique_ptr` / 値で表現。共有が必要な場合のみ `std::shared_ptr`。利用者に渡す参照はハンドル（無効化検出可能）。
- グローバル状態・シングルトンを公開しない。

### IV. テスト必須 (Test-First)
- 自動テスト（GoogleTest）が必須。公開 API の全メソッドに単体テストを書く。
- ウィンドウ不要なロジック（シーン、コンポーネント、シリアライズ、入力状態、当たり判定）はヘッドレスでテスト可能に設計する。
- CI 相当として `ctest` が全プリセットで成功することをマージ条件とする。

### V. ビルドの再現性
- CMake（3.25+）+ vcpkg マニフェストモード。Visual Studio 2022（v143）と Visual Studio 2026（v145）の両方で警告ゼロ（`/W4 /permissive-`）でビルドできること。
- `CMakePresets.json` に VS2022 / VS2026 用プリセットを用意する。

## 追加制約
- 言語: C++20（`std::span`, concepts, `std::format` 可。モジュール (`import`) は使わない）。
- 例外: 公開 API は例外を投げない。失敗は `bool` / `std::optional` / `std::expected` 相当の `sprout::Result<T>` で返す。
- コメント・ドキュメントは日本語可。識別子は英語。

## Governance
- 本規約は他の全ての慣習に優先する。変更には理由と移行方針を記した PR を要する。
- レビューでは命名規則と依存方向を必ず確認する。

**Version**: 1.0.0 | **Ratified**: 2026-10-21 | **Last Amended**: 2026-10-21
