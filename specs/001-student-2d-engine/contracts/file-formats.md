# Contract: ファイル形式

全テキスト形式は UTF-8 JSON、改行 LF、キー順安定（差分管理のため）、ルートに `formatVersion`。構造は ER 図 1 に従う。

## プロジェクト構成
```text
MyGame/
├── sprout.project.json        # PROJECT
├── ProjectSettings/*.json     # PROJECT_SETTINGS, TAG/LAYER, INPUT, AUDIO, BUILD_PROFILE
├── Assets/**                  # アセット + 同名 .meta
├── Game/CMakeLists.txt, src/  # ゲームモジュール
├── Library/                   # キャッシュ（VCS 対象外、削除しても再生成）
└── Build/                     # 書き出し先
```

## .meta
```json
{ "formatVersion": 1, "guid": "6f1c…", "importer": "TextureImporter", "importerVersion": 2,
  "settings": { "filter": "Point", "pixelsPerUnit": 16, "spriteMode": "Grid", "sprites": [ { "localId": 1, "name": "idle_0", "rect": [0,0,16,16], "pivot": [0.5,0] } ] } }
```

## .scene / .prefab
```json
{
  "formatVersion": 1, "type": "Scene", "name": "Stage1",
  "objects": [
    { "fileId": 100, "name": "Player", "active": true, "tag": "Player", "layer": 8, "parent": 0, "sibling": 0,
      "components": [
        { "fileId": 101, "type": "Transform", "localPosition": [0, 0], "localRotationDeg": 0, "localScale": [1, 1] },
        { "fileId": 102, "type": "SpriteRenderer", "enabled": true, "sprite": { "guid": "6f1c…", "localId": 1 } },
        { "fileId": 103, "type": "Behavior:PlayerMover", "enabled": true, "properties": { "m_speed": 200.0, "m_target": { "fileId": 200 } } }
      ] }
  ],
  "prefabInstances": [
    { "fileId": 300, "prefab": "a9e2…", "parent": 0,
      "overrides": [ { "target": 5, "path": "m_speed", "value": 120.0 } ], "added": [], "removedComponents": [] }
  ]
}
```
- 参照: アセット = `{guid, localId?}`、ファイル内オブジェクト = `{fileId}`、なし = `null`
- 未知の `type` は内容を保持したまま「Missing Script」として読込（保存時に失わない）

## その他アセット
`.anim`, `.animator`, `.tileset`, `.ruletile`, `.particle`, `.inputactions`, `.strings`, `.physmat`, `.material`, `.atlas` 。各スキーマは ER 図 1-D/1-E の属性名を camelCase キーとしてそのまま使う。

## .spak（書き出し）
`header{magic "SPAK", version, entryCount, tocOffset}` + zstd 圧縮エントリ（バイナリ化 JSON / 生アセット）+ TOC `{guid, offset, size, rawSize, hash}`

## セーブ
`%APPDATA%/<Company>/<Product>/save_<slot>.json` + `.bak`、`checksum`（xxHash64）不一致で `.bak` から復旧
