# Quickstart: 検証手順

## 前提
- Visual Studio 2022（17.10+）または Visual Studio 2026、「C++ によるデスクトップ開発」
- vcpkg を clone し環境変数 `VCPKG_ROOT` を設定
- CMake 4.2+（VS 同梱版で可）

## ビルドとテスト
```powershell
cmake --preset vs2022        # VS2026 は --preset vs2026
cmake --build --preset vs2022-debug
ctest --preset vs2022-debug   # 全テスト成功が期待値
cmake --build --preset vs2022-debug --target sprout_lint   # 命名規則違反 0 件
```

## シナリオ
| # | 手順 | 期待結果 | 対応 |
|---|---|---|---|
| 1 | `HelloSprout.exe` を起動し矢印キー | スプライトが移動 | US1 |
| 2 | `assets/player.png` を削除して起動 | 代替画像 + 日本語エラーログ、クラッシュなし | US1 Edge |
| 3 | `SproutEditor.exe` でオブジェクト 3 つ配置→保存→再起動→読み込み | 同じ配置 | US2 |
| 4 | エディタで移動→Ctrl+Z | 元の位置 | US2 |
| 5 | 再生→オブジェクトが動く→停止 | 再生前の位置に戻る | US3 |
| 6 | `TileCollector.exe` で壁へ移動 | 通り抜けない、アイテム取得でログ | US4 |
| 7 | 任意キーで効果音、起動時 BGM | 音が鳴る | US5 |

公開 API は [contracts/public-api.md](contracts/public-api.md)、データは [data-model.md](data-model.md) を参照。
