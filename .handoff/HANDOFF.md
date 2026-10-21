# 引き継ぎ (2026-10-21 / 001-student-2d-engine)

次の一手: Claude Code で `/pickup` を実行し、T027 以降の Phase 2 Foundational を roc の coder（並列・ファイル非重複）で続ける

## 完了
- Phase 1 Setup T001〜T020（CMake・プリセット・vcpkg・lint・CI）(50b5e80)
- Phase 2 math T021〜T026: Vector2/Vector2Int・Matrix3・Rect/Circle とテスト (`engine/include/sprout/math/`, `tests/unit/math/`, 50b5e80)
- 進捗: tasks 436 件中 26 件完了 (`specs/001-student-2d-engine/tasks.md`)
- roc coder が OpenCode + DeepSeek（`opencode/deepseek-v4.1-flash-free`, cost 0）で動くことを確認 (`.orchestrator/usage.jsonl`)

## 残り（優先順・最大5件）
- T027〜T134 Phase 2 Foundational（M0）
- Phase 3 US1 MVP（01_HelloSprout）
- Phase 4〜5 US2/US3（エディタ・ホットリロード）
- 各区切りで reviewer / tester による確認

## 保留
- VS2026 CI — 自前ランナーの用意待ち

## 再開に必要なもの
- Visual Studio 2022（17.10+）または 2026、「C++ によるデスクトップ開発」
- CMake 4.2+、vcpkg（環境変数 `VCPKG_ROOT`）
- `gh` CLI（ログイン済み）、`uvx`（Spec Kit 用）、Node.js（任意）
- git 管理外: `.orchestrator/`, `roc-free-1.cmd`（ローカルの roc 設定。free mode には `ROC_BACKEND=opencode` が必要）
- ビルド・テスト: `cmake --build --preset vs2022-debug` / `ctest --preset vs2022-debug`
