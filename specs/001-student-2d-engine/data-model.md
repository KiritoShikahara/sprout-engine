# Data Model: Sprout Engine

## Game
- `config: GameConfig { title: std::string, width: unsigned, height: unsigned, targetFps: unsigned = 60, startScene: std::string }`
- 状態遷移: `Created → Running → Stopping → Stopped`

## Scene
- `name: std::string`、`objects`（内部: `ObjectRegistry`）
- 検証: オブジェクト名は重複可、`findObject` は最初の一致を返す
- 生成/破棄要求は `pending` キューに入り、フレーム末に反映

## GameObject（利用者にはハンドル）
- 内部: `ObjectHandle { index: uint32, generation: uint32 }`
- `name`, `isActive`, `transform`, `components: vector<unique_ptr<Component>>`, `parent: ObjectHandle`, `children: vector<ObjectHandle>`
- 不変条件: 親子関係に循環なし、同種 Component は 1 つ（Behavior は型ごとに 1 つ）
- 状態: `Alive → PendingDestroy → Destroyed`（generation を進め、旧ハンドルは無効）

## Component（基底）
| 型 | フィールド |
|---|---|
| Transform | position: Vector2, rotationDegrees: float, scale: Vector2（ローカル）、worldPosition()（計算） |
| SpriteRenderer | texturePath, color: Color, layer: int, flipX/flipY: bool |
| TextRenderer | text, fontPath, characterSize: unsigned, color |
| BoxCollider | size: Vector2, offset: Vector2, isTrigger: bool, isStatic: bool |
| TileMap | tilesetPath, tileSize: Vector2u, columns, rows, tiles: vector<int>（-1 = 空）, solidTileIds: set<int> |
| AudioSource | clipPath, volume: 0–100, isLooping, playOnStart |
| Behavior | 利用者定義。`declareProperties` で公開した Property 群 |

## Property（エディタ公開値）
- `name`, `type ∈ {Int, Float, Bool, String, Color, Vector2}`, getter/setter

## Asset
- キー: 正規化した相対パス。`AssetCache<T>` が 1 回だけ読み込み（FR-007）。失敗時は `FallbackAssets`（紫黒チェッカー画像など）

## EditorCommand
- `execute()`, `undo()`, `label()`, `mergeWith(const EditorCommand&) -> bool`
- `CommandHistory { undoStack, redoStack, capacity = 200 }`

## SceneFile
- contracts/scene-file-format.md 参照
