# 第三者ライセンス表記

Sprout Engine は以下のオープンソースソフトウェアを vcpkg 経由で使用します。
各ライブラリの完全なライセンス条文は、vcpkg のインストール先
`<vcpkg>/installed/<triplet>/share/<port>/copyright` に含まれます。

| ライブラリ | 用途 | ライセンス |
|---|---|---|
| SFML | 描画・ウィンドウ・音声 | zlib |
| Box2D | 2D 物理 | MIT |
| Dear ImGui | エディタ UI | MIT |
| ImGui-SFML | ImGui と SFML の連携 | MIT |
| imnodes | ノードグラフ UI | MIT |
| nlohmann/json | JSON 読み書き | MIT |
| zstd | パック圧縮 | BSD-3-Clause |
| xxHash | ハッシュ・チェックサム | BSD-2-Clause |
| efsw | ファイル監視 | MIT |
| nativefiledialog-extended | ネイティブファイルダイアログ | zlib |
| Tracy | プロファイラ連携（任意） | BSD-3-Clause |
| GoogleTest | 自動テスト | BSD-3-Clause |
| ImGui Test Engine | エディタ E2E テスト | 独自ライセンス（非商用・教育利用は無償） |
| Noto Sans JP | エディタの日本語フォント | SIL Open Font License 1.1 |

## 注意

- ImGui Test Engine は MIT ではありません。ocornut/imgui_test_engine の
  `LICENSE.txt` に従い、教育・非商用利用の範囲で使用します。商用配布時は
  別途ライセンスを確認してください。
- 依存を追加・更新した場合は、この表と `vcpkg.json` を同時に更新してください。
