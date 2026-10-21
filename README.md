# Sprout Engine

> **これはサンプルです。** 専門学校の 1 年生向けに作った、学習用の 2D ゲームエンジン（エディタ付き）です。

SFML 3 と C++20 で作る、初学者向けのシンプルな 2D ゲームエンジンです。エンジン内部を知らなくても、短いコードでゲームを動かせることを目指しています。

## 対象
- 専門学校 1 年生（C++ の基礎を学び始めた人）
- Windows 10/11 + Visual Studio 2022 / Visual Studio 2026

## 特徴（予定）
- `GameObject` + `Component` 方式。独自の処理は `Behavior` を継承して `onUpdate()` を書くだけ
- ImGui 製エディタ（シーンツリー・インスペクタ・ビューポート・再生/停止・元に戻す）
- シーンは JSON で保存
- 当たり判定・タイルマップ・音

## 状態
現在は要件定義と設計の段階です。仕様は [specs/001-student-2d-engine/](specs/001-student-2d-engine/spec.md)、命名規則などの開発規約は [.specify/memory/constitution.md](.specify/memory/constitution.md) を参照してください。

## ビルド（実装後）
```powershell
cmake --preset vs2022        # VS2026 の場合は vs2026
cmake --build --preset vs2022-debug
ctest --preset vs2022-debug
```
