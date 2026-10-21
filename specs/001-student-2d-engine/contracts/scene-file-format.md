# Contract: シーンファイル形式（*.scene.json）

```json
{
  "formatVersion": 1,
  "name": "Main",
  "objects": [
    {
      "id": 1,
      "name": "Player",
      "active": true,
      "parentId": null,
      "components": [
        { "type": "Transform", "position": [100, 200], "rotationDegrees": 0, "scale": [1, 1] },
        { "type": "SpriteRenderer", "texturePath": "assets/player.png", "color": [255,255,255,255], "layer": 0 },
        { "type": "Behavior", "behaviorName": "PlayerMover", "properties": { "speed": 200.0 } }
      ]
    }
  ]
}
```

- `id` はファイル内のみ有効な整数（ハンドルとは別）
- 未知の `type` / `behaviorName` は警告ログを出してスキップ（読み込みは継続）
- `formatVersion` が現行より新しい場合は読み込みを拒否し、版を示すエラーを返す
- パスはプロジェクトルートからの相対、区切りは `/`
