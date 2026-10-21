# Contract: sprout_cli

| コマンド | 説明 | 終了コード |
|---|---|---|
| `sprout new <path> --template 2DEmpty` | プロジェクト作成 | 0 / 2 引数不正 / 3 既存 |
| `sprout build <project> --config Release` | ゲームモジュールをビルド | 0 / 10 コンパイル失敗 |
| `sprout export <project> --profile Default --out <dir>` | 書き出し | 0 / 20 アセットエラー / 21 書込失敗 |
| `sprout test <project>` | ゲーム側テストとアセット検証（Missing 参照検出） | 0 / 30 失敗 |
| `sprout validate <project>` | 形式・版・GUID 重複検査 | 0 / 40 |
| `sprout template register <project> --name …` | 教員用テンプレート登録 | 0 |

共通: `--json` で機械可読出力、`--log <file>`、標準エラーに日本語メッセージ。
