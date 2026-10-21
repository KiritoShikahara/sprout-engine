# ER 図: Sprout Engine

本書はデータ構造の正（FR-907）。構造を変える変更は、同じ変更内で本書を更新すること。
GitHub 上では Mermaid として図で表示される。

- **図 1 ドメイン（永続化データ）**: ファイルに保存されるもの。アセット・シーン・Prefab・設定
- **図 2 ランタイム内部**: 実行中のメモリ上の構造。`sprout::detail` 側。利用者からは見えない
- **図 3 エディタ**: エディタ固有の状態
- **付録**: 識別子の規則、カーディナリティの補足

記法: `||--o{` = 1 対 0..N、`||--|{` = 1 対 1..N、`||--||` = 1 対 1、`|o--o|` = 0..1 対 0..1。PK/FK/UK は主キー/外部キー/一意キー。

---

## 図 1. ドメイン（永続化データ）

### 1-A. プロジェクト・アセット基盤

```mermaid
erDiagram
    PROJECT ||--|| PROJECT_SETTINGS : has
    PROJECT ||--o{ BUILD_PROFILE : has
    PROJECT ||--|| ASSET_DATABASE : owns
    PROJECT ||--o{ GAME_MODULE : builds
    ASSET_DATABASE ||--o{ ASSET : indexes
    ASSET ||--|| ASSET_META : "described by"
    ASSET_META ||--o| IMPORT_SETTINGS : has
    ASSET ||--o{ ASSET_DEPENDENCY : "depends on (from)"
    ASSET ||--o{ ASSET_DEPENDENCY : "is used by (to)"
    ASSET ||--o{ SUB_ASSET : contains
    PROJECT_SETTINGS ||--o{ TAG_DEF : defines
    PROJECT_SETTINGS ||--o{ LAYER_DEF : defines
    PROJECT_SETTINGS ||--o{ SORTING_LAYER_DEF : defines
    PROJECT_SETTINGS ||--|| COLLISION_MATRIX : has
    PROJECT_SETTINGS ||--o| INPUT_ACTION_ASSET : "default input"
    PROJECT_SETTINGS ||--o{ AUDIO_MIXER_GROUP : defines
    BUILD_PROFILE ||--o{ BUILD_SCENE_ENTRY : includes
    BUILD_SCENE_ENTRY }o--|| ASSET : "scene asset"

    PROJECT {
        string name
        string engineVersion "semver"
        string rootPath
        string templateId
    }
    PROJECT_SETTINGS {
        float fixedTimeStep "default 0.02"
        float maxDeltaTime "default 0.25"
        vec2 gravity
        string defaultLocale
        int pixelsPerUnit
    }
    ASSET_DATABASE {
        int formatVersion
        datetime lastScan
    }
    ASSET {
        guid id PK
        string path UK "project relative"
        enum type "Texture,Audio,Font,Scene,Prefab,Material,Shader,AnimClip,Animator,TileSet,RuleTile,Particle,InputActions,StringTable,PhysicsMaterial,SpriteAtlas"
        hash contentHash
        datetime importedAt
    }
    ASSET_META {
        guid assetId PK, FK
        int metaVersion
        string importerName
        int importerVersion
    }
    IMPORT_SETTINGS {
        guid assetId PK, FK
        json values "type-specific"
    }
    SUB_ASSET {
        guid assetId FK
        int localId "unique in asset"
        enum type "Sprite,AnimEvent,..."
        string name
    }
    ASSET_DEPENDENCY {
        guid fromAsset FK
        guid toAsset FK
        enum kind "Hard,Soft"
    }
    TAG_DEF { int index PK "0..N" string name UK }
    LAYER_DEF { int index PK "0..31" string name UK }
    SORTING_LAYER_DEF { int order PK string name UK }
    COLLISION_MATRIX { bits32x32 mask }
    AUDIO_MIXER_GROUP {
        string name UK
        string parentName FK
        float volumeDb
        bool isMuted
    }
    BUILD_PROFILE {
        string name UK
        string productName
        string version
        guid iconTexture FK
        bool isDevelopmentBuild
        guid startScene FK
    }
    BUILD_SCENE_ENTRY { int order guid sceneAsset FK }
    GAME_MODULE {
        string name
        string binaryPath
        hash buildHash
        datetime builtAt
    }
```

### 1-B. シーン・オブジェクト・Prefab

```mermaid
erDiagram
    SCENE_ASSET ||--o{ GAME_OBJECT : contains
    PREFAB_ASSET ||--|{ GAME_OBJECT : "contains (root + children)"
    PREFAB_ASSET |o--o| PREFAB_ASSET : "variant of"
    GAME_OBJECT ||--|| TRANSFORM : has
    GAME_OBJECT ||--o{ COMPONENT : has
    GAME_OBJECT |o--o{ GAME_OBJECT : "parent of"
    GAME_OBJECT }o--o| PREFAB_INSTANCE : "belongs to"
    PREFAB_INSTANCE }o--|| PREFAB_ASSET : "instance of"
    PREFAB_INSTANCE ||--o{ PROPERTY_OVERRIDE : has
    PREFAB_INSTANCE ||--o{ ADDED_OBJECT_OVERRIDE : has
    PREFAB_INSTANCE ||--o{ REMOVED_COMPONENT_OVERRIDE : has
    COMPONENT ||--o{ SERIALIZED_PROPERTY : stores
    SERIALIZED_PROPERTY }o--o| ASSET : "asset reference"
    SERIALIZED_PROPERTY }o--o| GAME_OBJECT : "object reference"

    SCENE_ASSET {
        guid id PK
        int formatVersion
        string name
        color backgroundColor
    }
    PREFAB_ASSET {
        guid id PK
        int formatVersion
        int64 rootObjectFileId FK
        guid basePrefab FK "variant only"
    }
    GAME_OBJECT {
        int64 fileId PK "unique in file"
        string name
        bool isActive
        int tagIndex FK
        int layerIndex FK
        bool isStatic
        int64 parentFileId FK
        int siblingIndex
        int64 prefabInstanceFileId FK
    }
    TRANSFORM {
        int64 ownerFileId PK, FK
        vec2 localPosition
        float localRotationDeg
        vec2 localScale
    }
    COMPONENT {
        int64 fileId PK
        int64 ownerFileId FK
        string typeName "e.g. SpriteRenderer, Behavior:PlayerMover"
        bool isEnabled
        int order
    }
    SERIALIZED_PROPERTY {
        int64 componentFileId FK
        string path "e.g. color.r, items[2]"
        variant value
    }
    PREFAB_INSTANCE {
        int64 fileId PK
        guid prefab FK
    }
    PROPERTY_OVERRIDE {
        int64 instanceFileId FK
        int64 targetFileId "id inside prefab"
        string propertyPath
        variant value
    }
    ADDED_OBJECT_OVERRIDE { int64 instanceFileId FK int64 addedFileId FK int64 attachParentFileId }
    REMOVED_COMPONENT_OVERRIDE { int64 instanceFileId FK int64 targetComponentFileId }
```

### 1-C. コンポーネント型（COMPONENT のサブタイプ）

```mermaid
erDiagram
    COMPONENT ||--o| SPRITE_RENDERER : "is a"
    COMPONENT ||--o| TEXT_RENDERER : "is a"
    COMPONENT ||--o| SHAPE_RENDERER : "is a"
    COMPONENT ||--o| CAMERA : "is a"
    COMPONENT ||--o| LIGHT_2D : "is a"
    COMPONENT ||--o| RIGIDBODY_2D : "is a"
    COMPONENT ||--o| COLLIDER_2D : "is a"
    COMPONENT ||--o| JOINT_2D : "is a"
    COMPONENT ||--o| ANIMATOR : "is a"
    COMPONENT ||--o| AUDIO_SOURCE : "is a"
    COMPONENT ||--o| AUDIO_LISTENER : "is a"
    COMPONENT ||--o| PARTICLE_SYSTEM : "is a"
    COMPONENT ||--o| TILEMAP : "is a"
    COMPONENT ||--o| UI_ELEMENT : "is a"
    COMPONENT ||--o| BEHAVIOR : "is a"
    SPRITE_RENDERER }o--o| SPRITE : uses
    SPRITE_RENDERER }o--o| MATERIAL : uses
    COLLIDER_2D }o--o| PHYSICS_MATERIAL : uses
    JOINT_2D }o--o| RIGIDBODY_2D : "connected body"
    ANIMATOR }o--|| ANIMATOR_CONTROLLER : uses
    AUDIO_SOURCE }o--o| AUDIO_CLIP : plays
    AUDIO_SOURCE }o--o| AUDIO_MIXER_GROUP : "outputs to"
    TILEMAP }o--|| TILESET : uses
    TILEMAP ||--o{ TILE_LAYER : has
    BEHAVIOR }o--|| BEHAVIOR_TYPE : "instance of"

    SPRITE_RENDERER { guid sprite FK color tint bool flipX bool flipY enum drawMode "Simple,Sliced,Tiled" string sortingLayer int orderInLayer guid material FK }
    TEXT_RENDERER { string text guid font FK int size color color enum alignment bool isRichText float outline }
    SHAPE_RENDERER { enum shape "Line,Rect,Circle,Polygon" vec2list points color fill color stroke float strokeWidth }
    CAMERA { float zoom color background bits32 cullingMask rect viewport guid renderTarget FK int depth }
    LIGHT_2D { enum type "Point,Spot,Global" color color float intensity float radius float angle bool castsShadow }
    RIGIDBODY_2D { enum bodyType "Dynamic,Kinematic,Static" float mass float gravityScale float linearDamping float angularDamping bool fixedRotation bool isBullet bool canSleep }
    COLLIDER_2D { enum shape "Box,Circle,Capsule,Polygon,Edge,Composite" vec2 offset json shapeData bool isTrigger guid material FK }
    JOINT_2D { enum type "Distance,Hinge,Slider,Spring,Fixed,Target" int64 connectedBody FK json params }
    ANIMATOR { guid controller FK float speed enum updateMode "Normal,Unscaled,Physics" }
    AUDIO_SOURCE { guid clip FK float volume float pitch bool isLooping bool playOnStart int priority bool isSpatial float minDistance float maxDistance }
    AUDIO_LISTENER { float volume }
    PARTICLE_SYSTEM { guid effect FK bool playOnStart enum simulationSpace "Local,World" }
    TILEMAP { guid tileset FK vec2 cellSize enum colliderMode "None,PerTile,Composite" }
    TILE_LAYER { string name int order int32array cells "tile id or -1" int columns int rows }
    UI_ELEMENT { enum kind "Canvas,Panel,Image,Text,Button,Toggle,Slider,ProgressBar,InputField,Dropdown,ScrollView,Mask,LayoutGroup" vec4 anchors vec2 pivot vec4 offsets json kindData }
    BEHAVIOR { string typeName FK }
```

### 1-D. 表現系アセット（描画・アニメーション・物理・音・タイル）

```mermaid
erDiagram
    TEXTURE ||--o{ SPRITE : "sliced into"
    SPRITE_ATLAS }o--o{ SPRITE : packs
    MATERIAL }o--|| SHADER : uses
    MATERIAL ||--o{ MATERIAL_PARAM : has
    ANIMATION_CLIP ||--o{ ANIMATION_TRACK : has
    ANIMATION_TRACK ||--|{ KEYFRAME : has
    ANIMATION_CLIP ||--o{ ANIMATION_EVENT : has
    ANIMATOR_CONTROLLER ||--|{ ANIMATOR_LAYER : has
    ANIMATOR_CONTROLLER ||--o{ ANIMATOR_PARAMETER : declares
    ANIMATOR_LAYER ||--|| STATE_MACHINE : "root"
    STATE_MACHINE ||--o{ ANIMATOR_STATE : contains
    STATE_MACHINE |o--o{ STATE_MACHINE : "sub machine"
    ANIMATOR_STATE }o--o| ANIMATION_CLIP : plays
    ANIMATOR_STATE ||--o{ ANIMATOR_TRANSITION : "from"
    ANIMATOR_TRANSITION }o--|| ANIMATOR_STATE : "to"
    ANIMATOR_TRANSITION ||--o{ TRANSITION_CONDITION : has
    TRANSITION_CONDITION }o--|| ANIMATOR_PARAMETER : tests
    TILESET }o--|| TEXTURE : uses
    TILESET ||--|{ TILE_DEF : defines
    TILE_DEF ||--o{ TILE_FRAME : "animated by"
    RULE_TILE ||--|{ TILE_RULE : has
    TILE_RULE }o--|| TILE_DEF : outputs
    PARTICLE_EFFECT ||--|{ PARTICLE_MODULE : has
    PARTICLE_EFFECT }o--o| MATERIAL : renders

    TEXTURE { guid id PK int width int height enum filter "Point,Bilinear" bool isRepeated bool generateMipmaps int pixelsPerUnit }
    SPRITE { int localId PK rect sourceRect vec2 pivot vec4 border "9-slice" }
    SPRITE_ATLAS { guid id PK int maxSize int padding }
    SHADER { guid id PK string vertexPath string fragmentPath }
    MATERIAL { guid id PK enum blendMode "Alpha,Additive,Multiply" }
    MATERIAL_PARAM { string name enum type "Float,Vec2,Color,Texture" variant value }
    ANIMATION_CLIP { guid id PK float length float frameRate bool isLooping }
    ANIMATION_TRACK { string targetPath "child path" string property enum interpolation "Constant,Linear,Bezier" }
    KEYFRAME { float time variant value vec2 inTangent vec2 outTangent }
    ANIMATION_EVENT { float time string functionName variant argument }
    ANIMATOR_CONTROLLER { guid id PK }
    ANIMATOR_LAYER { string name float weight int order }
    STATE_MACHINE { string name int64 defaultState FK }
    ANIMATOR_STATE { int64 id PK string name float speed bool isAnyState }
    ANIMATOR_TRANSITION { int64 fromState FK int64 toState FK float duration bool hasExitTime float exitTime }
    ANIMATOR_PARAMETER { string name UK enum type "Float,Int,Bool,Trigger" variant defaultValue }
    TRANSITION_CONDITION { string parameter FK enum op "Greater,Less,Equals,NotEquals,If,IfNot" variant threshold }
    PHYSICS_MATERIAL { guid id PK float friction float bounciness }
    AUDIO_CLIP { guid id PK float duration int channels int sampleRate enum loadMode "Preload,Stream" }
    FONT { guid id PK string family }
    TILESET { guid id PK vec2 tileSize int spacing int margin }
    TILE_DEF { int tileId PK int spriteIndex enum collider "None,Full,Custom" vec2list customShape }
    TILE_FRAME { int tileId FK int spriteIndex float duration }
    RULE_TILE { guid id PK guid tileset FK }
    TILE_RULE { int order u8x8 neighborMask int outputTileId FK }
    PARTICLE_EFFECT { guid id PK float duration bool isLooping int maxParticles }
    PARTICLE_MODULE { enum kind "Emission,Shape,Velocity,Force,ColorOverLife,SizeOverLife,RotationOverLife,TextureAnim,Collision" json params }
```

### 1-E. 入力・多言語・セーブ・スクリプト型情報

```mermaid
erDiagram
    INPUT_ACTION_ASSET ||--|{ INPUT_ACTION_MAP : has
    INPUT_ACTION_MAP ||--|{ INPUT_ACTION : has
    INPUT_ACTION ||--o{ INPUT_BINDING : "bound by"
    INPUT_BINDING ||--o{ INPUT_BINDING : "composite parts"
    INPUT_BINDING ||--o{ INPUT_PROCESSOR : has
    PLAYER_PREFS ||--o{ BINDING_OVERRIDE : stores
    BINDING_OVERRIDE }o--|| INPUT_BINDING : overrides
    STRING_TABLE ||--o{ STRING_ENTRY : has
    STRING_ENTRY ||--|{ LOCALIZED_VALUE : "per locale"
    LOCALE ||--o{ LOCALIZED_VALUE : "language of"
    LOCALE }o--o| FONT : "fallback font"
    SAVE_SLOT ||--o{ SAVE_RECORD : contains
    SAVE_SLOT ||--o| SAVE_SLOT : "backup of"
    GAME_MODULE ||--o{ BEHAVIOR_TYPE : exports
    BEHAVIOR_TYPE ||--o{ PROPERTY_DESCRIPTOR : declares
    PROPERTY_DESCRIPTOR ||--o{ PROPERTY_ATTRIBUTE : has
    GAME_MODULE ||--o{ CUSTOM_INSPECTOR : exports
    CUSTOM_INSPECTOR }o--|| BEHAVIOR_TYPE : "draws"

    INPUT_ACTION_ASSET { guid id PK }
    INPUT_ACTION_MAP { string name UK bool isEnabledByDefault }
    INPUT_ACTION { string name UK enum valueType "Button,Axis1D,Axis2D" }
    INPUT_BINDING { int64 id PK enum device "Keyboard,Mouse,Gamepad" string control "e.g. Key.Space" enum composite "None,Axis1D,Dpad" string partName }
    INPUT_PROCESSOR { enum kind "Deadzone,Scale,Invert,Normalize" float param }
    BINDING_OVERRIDE { int64 bindingId FK string control }
    PLAYER_PREFS { float masterVolume float bgmVolume float seVolume string locale bool isFullscreen vec2 resolution }
    STRING_TABLE { guid id PK string name }
    STRING_ENTRY { string key UK string comment }
    LOCALE { string code PK "ja, en" string displayName }
    LOCALIZED_VALUE { string key FK string localeCode FK string text }
    SAVE_SLOT { int index PK int dataVersion datetime savedAt hash checksum }
    SAVE_RECORD { string key json value }
    BEHAVIOR_TYPE { string name PK int version int executionOrder string category }
    PROPERTY_DESCRIPTOR { string name enum type "Int,Float,Bool,String,Color,Vec2,Enum,AssetRef,ObjectRef,ComponentRef,Array" int offset }
    PROPERTY_ATTRIBUTE { enum kind "Range,Tooltip,Header,Hidden,ReadOnly,Multiline" json params }
    CUSTOM_INSPECTOR { string targetType FK }
```

---

## 図 2. ランタイム内部（sprout::detail）

利用者に渡るのは `GameObject` / `Ref<T>` という**ハンドル（index + generation）**だけ。実体は以下のテーブル群に格納される。

```mermaid
erDiagram
    ENGINE ||--|| WORLD_MANAGER : owns
    ENGINE ||--|| ASSET_MANAGER : owns
    ENGINE ||--|| RENDER_SYSTEM : owns
    ENGINE ||--|| PHYSICS_SYSTEM : owns
    ENGINE ||--|| AUDIO_SYSTEM : owns
    ENGINE ||--|| INPUT_SYSTEM : owns
    ENGINE ||--|| JOB_SYSTEM : owns
    ENGINE ||--|| TIME_SYSTEM : owns
    ENGINE ||--|| EVENT_BUS : owns
    ENGINE ||--|| MODULE_LOADER : owns
    ENGINE ||--|| PROFILER : owns
    MODULE_LOADER ||--o| LOADED_MODULE : "current"
    LOADED_MODULE ||--o{ TYPE_INFO : registers
    WORLD_MANAGER ||--|{ WORLD : "loaded scenes"
    WORLD ||--|| OBJECT_TABLE : has
    WORLD ||--o{ COMPONENT_POOL : "one per type"
    WORLD ||--|| PENDING_QUEUE : has
    WORLD ||--|| HIERARCHY_INDEX : has
    OBJECT_TABLE ||--o{ OBJECT_SLOT : contains
    OBJECT_SLOT ||--o{ COMPONENT_SLOT_REF : "component list"
    COMPONENT_POOL ||--o{ COMPONENT_SLOT : stores
    COMPONENT_SLOT_REF }o--|| COMPONENT_SLOT : points
    COMPONENT_POOL }o--|| TYPE_INFO : "typed by"
    TYPE_INFO ||--o{ FIELD_INFO : has
    ASSET_MANAGER ||--o{ ASSET_SLOT : caches
    ASSET_SLOT }o--|| ASSET_LOADER : "loaded by"
    RENDER_SYSTEM ||--o{ RENDER_QUEUE_ITEM : "per frame"
    RENDER_SYSTEM ||--o{ DRAW_BATCH : builds
    RENDER_QUEUE_ITEM }o--|| COMPONENT_SLOT : "from renderer"
    PHYSICS_SYSTEM ||--o{ BODY_PROXY : maps
    BODY_PROXY ||--|| COMPONENT_SLOT : "rigidbody"
    PHYSICS_SYSTEM ||--o{ CONTACT_RECORD : "per step"
    AUDIO_SYSTEM ||--o{ VOICE : "active voices"
    VOICE }o--|| ASSET_SLOT : "clip"
    EVENT_BUS ||--o{ SUBSCRIPTION : has
    SUBSCRIPTION }o--o| OBJECT_SLOT : "auto-unsubscribe owner"
    TIME_SYSTEM ||--o{ TIMER : schedules
    TIME_SYSTEM ||--o{ COROUTINE : runs
    TIMER }o--|| OBJECT_SLOT : "owner"
    COROUTINE }o--|| COMPONENT_SLOT : "owner"
    PROFILER ||--o{ PROFILE_FRAME : "ring buffer"
    PROFILE_FRAME ||--o{ PROFILE_SAMPLE : has

    ENGINE { enum state "Created,Running,Paused,Stopping" EngineConfig config }
    WORLD { uint32 worldId PK guid sceneAsset FK bool isPersistent }
    OBJECT_TABLE { uint32 capacity uint32 freeListHead }
    OBJECT_SLOT { uint32 index PK uint32 generation string name bool isActiveSelf uint32 tag uint32 layer uint32 parentIndex uint32 firstChild uint32 nextSibling enum lifeState "Alive,PendingDestroy,Free" }
    COMPONENT_POOL { uint32 typeId FK uint32 count "dense array" }
    COMPONENT_SLOT { uint32 index PK uint32 generation uint32 ownerIndex FK bool isEnabled bool hasStarted }
    COMPONENT_SLOT_REF { uint32 typeId uint32 slotIndex }
    PENDING_QUEUE { list toCreate list toDestroy list toAddComponent list toRemoveComponent }
    HIERARCHY_INDEX { dirtyset transformsDirty list rootObjects }
    TYPE_INFO { uint32 typeId PK string name uint32 size int executionOrder ptr factory ptr serializer }
    FIELD_INFO { string name enum type uint32 offset json attributes }
    LOADED_MODULE { string path hash buildHash uint32 reloadCount }
    ASSET_SLOT { guid assetId PK uint32 refCount enum state "Unloaded,Loading,Loaded,Failed" bool isFallback }
    ASSET_LOADER { string typeName }
    RENDER_QUEUE_ITEM { uint64 sortKey "layer|order|material|texture" uint32 componentIndex }
    DRAW_BATCH { uint32 textureId uint32 materialId uint32 vertexStart uint32 vertexCount }
    BODY_PROXY { uint64 nativeBodyId uint32 componentIndex FK }
    CONTACT_RECORD { uint32 a uint32 b enum phase "Enter,Stay,Exit" vec2 point vec2 normal bool isTrigger }
    VOICE { uint32 id PK uint32 sourceComponent FK int priority float volume }
    SUBSCRIPTION { uint32 id PK uint32 eventTypeId uint32 ownerIndex FK }
    TIMER { uint32 id PK float remaining float interval bool useUnscaled uint32 ownerIndex FK }
    COROUTINE { uint32 id PK enum waitKind "Seconds,NextFrame,Until,FixedUpdate" uint32 ownerComponent FK }
    PROFILE_FRAME { uint64 frameNumber float totalMs uint32 drawCalls uint64 memoryBytes }
    PROFILE_SAMPLE { string marker float ms uint8 depth }
```

### ハンドル規則
- `GameObject{ worldId, index, generation }` / `Ref<TComponent>{ worldId, typeId, index, generation }`
- 参照解決時に `generation` が一致しなければ無効（FR-104）。スロット再利用時に generation を加算
- 永続化時はハンドルではなく `fileId`（図 1）に変換。読み込み時に `fileId → index` の対応表で再構築

---

## 図 3. エディタ（sprout::editor）

```mermaid
erDiagram
    EDITOR_SESSION ||--|| OPENED_PROJECT : edits
    EDITOR_SESSION ||--|| SELECTION : has
    EDITOR_SESSION ||--|| COMMAND_HISTORY : has
    EDITOR_SESSION ||--|| PLAY_MODE_STATE : has
    EDITOR_SESSION ||--|| EDITOR_LAYOUT : uses
    EDITOR_SESSION ||--|| EDITOR_PREFERENCES : uses
    EDITOR_SESSION ||--o{ EDITOR_WINDOW : opens
    EDITOR_SESSION ||--o{ SCENE_DOCUMENT : "open documents"
    SCENE_DOCUMENT }o--|| WORLD : "edit world"
    SCENE_DOCUMENT ||--o| AUTOSAVE_SNAPSHOT : has
    COMMAND_HISTORY ||--o{ EDITOR_COMMAND : "undo/redo stacks"
    EDITOR_COMMAND ||--o{ EDITOR_COMMAND : "composite children"
    SELECTION ||--o{ SELECTED_ITEM : contains
    PLAY_MODE_STATE ||--o| PLAY_SNAPSHOT : "restore point"
    PLAY_MODE_STATE }o--o| WORLD : "play world"
    EDITOR_WINDOW }o--o| CUSTOM_EDITOR_TOOL : "if extension"
    BUILD_JOB }o--|| BUILD_PROFILE : uses
    EDITOR_SESSION ||--o{ BUILD_JOB : runs
    EDITOR_SESSION ||--o{ COMPILE_DIAGNOSTIC : "from module build"

    EDITOR_SESSION { string engineVersion bool isDirty }
    OPENED_PROJECT { string rootPath guid lastScene }
    SELECTION { enum context "Scene,Asset,PrefabMode" int64 activeItem }
    SELECTED_ITEM { enum kind "GameObject,Asset,AnimatorState,Tile" variant id }
    COMMAND_HISTORY { int capacity "default 500" int savePoint }
    EDITOR_COMMAND { string label datetime time int64 mergeKey enum kind "Create,Delete,SetProperty,Reparent,AddComponent,RemoveComponent,PaintTiles,ApplyOverride,Composite" }
    PLAY_MODE_STATE { enum state "Edit,Playing,Paused,Stepping" }
    PLAY_SNAPSHOT { bytes serializedWorlds list keptProperties }
    SCENE_DOCUMENT { guid sceneAsset FK bool isDirty enum mode "Scene,PrefabEdit" }
    AUTOSAVE_SNAPSHOT { string path datetime savedAt }
    EDITOR_LAYOUT { string name string imguiIni }
    EDITOR_PREFERENCES { enum theme "Dark,Light" enum language "ja,en" float fontScale int autosaveSeconds json shortcuts }
    EDITOR_WINDOW { string typeName bool isOpen }
    CUSTOM_EDITOR_TOOL { string menuPath string typeName }
    BUILD_JOB { enum state "Queued,Compiling,Packing,Done,Failed" string outputPath }
    COMPILE_DIAGNOSTIC { enum severity string file int line string message }
```

---

## 付録 A. 識別子の規則
| 識別子 | 範囲 | 永続 | 用途 |
|---|---|---|---|
| `guid` | 全プロジェクトで一意 | ○ | アセット参照 |
| `localId` | アセット内で一意 | ○ | サブアセット（スプライト等） |
| `fileId` | シーン/Prefab ファイル内で一意 | ○ | オブジェクト/コンポーネント参照の保存 |
| `index + generation` | ワールド内・実行中のみ | × | ハンドル（利用者が使う参照） |
| `typeId` | 実行中のみ（モジュール再読込で再採番） | × | 型の識別。保存時は `typeName` |

## 付録 B. 主な整合性制約
- GAME_OBJECT の親子は木構造（循環禁止）。PREFAB_ASSET の入れ子は DAG（循環禁止）
- 1 GAME_OBJECT に TRANSFORM は必ず 1 つ。同一型コンポーネントの重複は型ごとに許可/禁止を TYPE_INFO で定義（Rigidbody / Animator は禁止）
- PROPERTY_OVERRIDE.targetFileId は参照先 PREFAB_ASSET に存在すること（無ければ「孤立上書き」として警告し保持）
- ASSET.path の変更は ASSET.id を変えない
- LAYER_DEF.index は 0–31、COLLISION_MATRIX は対称
