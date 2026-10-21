# Contributing

Sprout Engine へのコントリビューション手順です。開発規約の最終的な根拠は
[.specify/memory/constitution.md](.specify/memory/constitution.md) です。

## 前提環境

- Visual Studio 2022（17.10 以降）または Visual Studio 2026、「C++ によるデスクトップ開発」
- CMake 3.25 以降（推奨 4.2 以降）
- vcpkg（環境変数 `VCPKG_ROOT` を設定し、マニフェストモードで使用）

## ビルドと確認

```powershell
cmake --preset vs2022
cmake --build --preset vs2022-debug
ctest --preset vs2022-debug
cmake --build --preset vs2022-debug --target sprout_lint
```

VS2026 では `vs2022` を `vs2026` に置き換えてください。

## ブランチ

- `main` へ直接 push しない。
- 作業ブランチは `<type>/<short-summary>`（例: `feat/scene-hierarchy`, `fix/transform-dirty`）。
- 仕様単位のブランチは `001-student-2d-engine` のようにスペック ID を使う。

## コミット

Conventional Commits を使います。

```text
<type>(<scope>): <summary>

<body>
```

- `type`: `feat`, `fix`, `refactor`, `test`, `docs`, `build`, `ci`, `chore`
- `summary` は命令形・日本語可・72 文字以内
- 1 コミット 1 目的。無関係な整形を含めない。

## Pull Request

PR テンプレートのチェックリストを埋めてください。以下を満たさない PR はマージしません。

- 変更に対応するテストがあり、`ctest --preset <preset>` が成功する
- `/W4 /permissive- /WX` で警告ゼロ
- `sprout_lint` 違反ゼロ
- 公開 API は日本語 Doxygen コメントと使用例を持つ
- データ構造を変更した場合は `specs/001-student-2d-engine/er-diagram.md` を更新する
- レビューを 1 件以上通過する

## 命名規則

[constitution の II. 命名規則](.specify/memory/constitution.md) に従います。
`.clang-tidy` の `readability-identifier-naming` で機械的に検査されます。

## 依存関係

- 新しい依存を追加するときは `vcpkg.json` に追加し、`builtin-baseline` を更新する。
- 公開ヘッダに外部ライブラリの型を露出しない。
