# Research: Sprout Engine

## R1. SFML バージョン
- Decision: SFML 3.0（vcpkg `sfml`）
- Rationale: C++17/20 前提の新 API（`sf::Vector2` 改善、イベントの `std::variant` 化）。今後の授業で長く使える
- Alternatives: SFML 2.6（情報は多いが旧 API、将来性で劣る）

## R2. エディタ UI
- Decision: Dear ImGui（docking ブランチ）+ imgui-sfml
- Rationale: パネル構成・ドッキングを短コードで実現。SFML 3 対応版が vcpkg にある
- Alternatives: Qt（重い・ライセンス・学習コスト）、自前 UI（工数過大）

## R3. エンティティ設計
- Decision: GameObject + Component（Unity 風）。内部はスロット配列 + 世代番号付きハンドル `ObjectHandle{index, generation}`
- Rationale: 学生に馴染みがあり、ハンドルで破棄済み参照を検出できる（Edge Case）。純 ECS は学習コストが高い
- Alternatives: EnTT による純 ECS（高速だが利用者に内部構造が見える）

## R4. 利用者振る舞いのエディタ登録（リフレクション）
- Decision: `SPROUT_REGISTER_BEHAVIOR(MyBehavior)` マクロ 1 行 + `void declareProperties(PropertyList& list)` で公開プロパティを宣言
- Rationale: C++20 に静的リフレクションが無いため、最小の手書きで済む方式。constitution のマクロ原則禁止に対し、唯一の例外として明記
- Alternatives: コード生成ツール（ビルドが複雑化）、全手書き登録（利用者負担大）

## R5. シーン形式
- Decision: JSON（nlohmann-json）、ルートに `formatVersion`
- Rationale: 人が読めて差分管理しやすい（FR-011）
- Alternatives: YAML（依存追加）、バイナリ（読めない）

## R6. Undo/Redo
- Decision: Command パターン（`EditorCommand::execute/undo`、`CommandHistory` 上限 100+）。値変更は旧値/新値を保持、連続ドラッグは `mergeWith` で 1 手に統合
- Alternatives: シーン全体スナップショット（単純だがメモリ大）

## R7. プレイモード
- Decision: 再生時にシーンを JSON へシリアライズして保存 → 実行 → 停止時にデシリアライズで復元
- Rationale: シリアライザを再利用でき、往復テストがそのまま品質保証になる

## R8. 固定時間刻み
- Decision: 物理 50 Hz 固定刻み + 描画可変、1 フレームの最大経過時間 0.25 s でクランプ

## R9. VS2022 / VS2026 両対応
- Decision: `CMakePresets.json` に generator `Visual Studio 17 2022` と `Visual Studio 18 2026` のプリセット、`CMAKE_TOOLCHAIN_FILE` は `$env{VCPKG_ROOT}`。CMake 4.2+ を推奨（VS2026 generator 対応）
- Alternatives: Ninja のみ（VS の開発者に馴染みが薄い。補助プリセットとして `ninja-msvc` も提供）

## R10. 命名規則の自動検査
- Decision: clang-tidy `readability-identifier-naming`（constitution II の表をそのまま設定）+ clang-format。CMake ターゲット `sprout_lint`
