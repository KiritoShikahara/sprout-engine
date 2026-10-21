# 引き継ぎ (2026-10-21 14:57 / 001-student-2d-engine)

次の一手: Claude Code で「T001〜T020 で開始」と指示し、roc の coder ロールで Phase 1（ビルド基盤）の実装を始める

## 完了
- Spec Kit 導入と constitution v1.2.0（命名規則・製品品質）(`.specify/memory/constitution.md`)
- 完全仕様 v2: US 15 本・FR 119 件・SC 12 件 (`specs/001-student-2d-engine/spec.md`, ca1c765)
- ER 図 7 枚（mmdc で描画検証済み）(`specs/001-student-2d-engine/er-diagram.md`)
- plan v2 一式: plan / research / data-model / contracts 5 本 / quickstart (02f8f99, 524db78)
- tasks v2: 436 件 (`specs/001-student-2d-engine/tasks.md`, f0cd55c)
- 公開リポジトリ https://github.com/KiritoShikahara/sprout-engine（main + 001-student-2d-engine）

## 残り（優先順・最大5件）
- T001〜T020 Phase 1 Setup（CMake・プリセット・vcpkg・lint・CI）
- T021〜T134 Phase 2 Foundational（M0）
- Phase 3 US1 MVP（01_HelloSprout）
- Phase 4〜5 US2/US3（エディタ・ホットリロード）
- 各区切りで reviewer / tester による確認

## 保留
- 実装担当モデル — ユーザーは opencode の DeepSeek を想定していたが、roc の coder は sonnet 定義。DeepSeek 割当の設定場所をユーザーに確認待ち
- VS2026 CI — 自前ランナーの用意待ち

## 再開に必要なもの
- Visual Studio 2022（17.10+）または 2026、「C++ によるデスクトップ開発」
- CMake 4.2+、vcpkg（環境変数 `VCPKG_ROOT`）
- `gh` CLI（ログイン済み）、`uvx`（Spec Kit 用）、Node.js（mermaid-cli 検証用・任意）
- git 管理外: `.orchestrator/`, `roc-free-1.cmd`（ローカルの roc 設定。必要なら別途コピー）
