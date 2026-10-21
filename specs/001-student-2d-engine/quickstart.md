# Quickstart: 検証手順（spec v2）

## 前提
- Visual Studio 2022（17.10+）または 2026、「C++ によるデスクトップ開発」
- `VCPKG_ROOT` 設定済み、CMake 4.2+

## エンジンのビルドと全テスト
```powershell
cmake --preset vs2022                       # or vs2026
cmake --build --preset vs2022-debug
ctest --preset vs2022-debug                 # unit / integration / determinism / editor_e2e
cmake --build --preset vs2022-debug --target sprout_lint   # 命名規則違反 0
ctest --preset vs2022-release -L perf       # SC-005 ベンチ
```

## シナリオ（各 US の受入確認）
| # | 手順 | 期待 | US |
|---|---|---|---|
| 1 | `samples/01_HelloSprout` を実行し WASD | 移動。画像削除時は代替 + 日本語エラー | 1 |
| 2 | `SproutEditor` → ハブで 2DEmpty 作成 → 3 体配置 → 保存 → 再起動 | 同一状態、Undo 200 手 | 2 |
| 3 | VS で Behavior 追加→ビルド（エディタ起動のまま）→再生→停止 | 10 秒以内に反映、停止で復元 | 3 |
| 4 | Behavior 内で null 参照をわざと起こす | エディタは落ちず再生停止・復元 | 3 |
| 5 | `03_Platformer` を開き再生 | 待機/歩き/ジャンプ遷移、壁で停止、コイン取得 | 4,5,7 |
| 6 | 敵 Prefab の速度変更 | 全インスタンス反映、個別上書き保持 | 6 |
| 7 | `04_Shooter` をタイトルから最後まで | 遷移・スコア引継ぎ・UI パッド操作・BGM クロスフェード | 8,9,10 |
| 8 | 設定画面でキー変更・言語切替→再起動 | 復元される | 10,11 |
| 9 | ビルド設定→書き出し→別 PC（VS 無し）で起動 | 起動する | 12 |
| 10 | 再生中にプロファイラを開く | システム別時間・描画呼出し数が見える | 13 |
| 11 | `sprout build` / `sprout export` をコマンドラインで | エディタ無しで成功 | 14 |
| 12 | カスタムインスペクタを登録 | 独自 UI 表示 | 15 |

参照: [contracts/](contracts/) / [data-model.md](data-model.md) / [er-diagram.md](er-diagram.md)
