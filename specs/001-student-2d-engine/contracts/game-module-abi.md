# Contract: ゲームモジュール ABI

## 成果物
- 学生プロジェクト `Game/` を `sprout_add_game_module(<Name> SOURCES ...)`（cmake/SproutGameModule.cmake）でビルド → `<Name>.dll` + `.pdb`
- リンク先は `SproutEngine.dll` の導入ライブラリのみ

## 入口
```cpp
extern "C" __declspec(dllexport) sprout::detail::ModuleExports* sproutModuleEntry(const sprout::detail::ModuleHostInfo* host);
```
`SPROUT_GAME_MODULE(Name)` が生成。`ModuleHostInfo { uint32 abiVersion; uint32 engineVersion; BuildConfig config; }`、`ModuleExports { uint32 abiVersion; const char* name; void (*registerTypes)(TypeRegistrar&); void (*unregisterTypes)(TypeRegistrar&); }`

## 互換性検査（読込時）
| 項目 | 不一致時 |
|---|---|
| `abiVersion` | 拒否「エンジン版が違います。プロジェクトを開き直して再ビルドしてください」 |
| Debug/Release | 拒否（CRT 不一致） |
| MSVC 主版（v14x） | 拒否 |

## 再読込手順（エディタ）
1. efsw が出力 DLL 更新を検知（書込完了まで 300 ms 待機）
2. 新 DLL を `Library/ModuleCache/<hash>/` へコピーし、互換性検査
3. 全ワールドを `WorldSnapshot` 化 → モジュール由来の全インスタンスを破棄（`onDestroy` は呼ばない）→ `unregisterTypes` → `FreeLibrary`
4. 新 DLL を `LoadLibrary` → `registerTypes` → スナップショット復元（フィールド名 + 型一致で移行、不一致は Warning）
5. 失敗時は前版 DLL を再ロードして継続（US3-6）

## 制約（学生向け、テンプレートに明記）
- static 変数の状態は再読込で失われる（保持したい値は `SPROUT_PROPERTY` かセーブへ）
- モジュール内でスレッドを立てない（リンター警告）
