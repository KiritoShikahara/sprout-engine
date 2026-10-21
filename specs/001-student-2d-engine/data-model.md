# Data Model: Sprout Engine（spec v2）

**データ構造の正は [er-diagram.md](er-diagram.md)**（FR-907）。本書は ER の各エンティティを C++ の型・モジュール・実装方針に対応付け、検証規則と状態遷移を定める。

## 1. ER → 型の対応

### 1.1 永続化（図 1）
| ER エンティティ | 公開型（sprout） | 内部型（sprout::detail） | モジュール |
|---|---|---|---|
| PROJECT / PROJECT_SETTINGS | — | `ProjectDescriptor`, `ProjectSettings` | editor_core / core |
| ASSET / ASSET_META / IMPORT_SETTINGS | `AssetRef<T>` | `AssetRecord`, `AssetMeta`, `ImportSettings`（型別 struct を `std::variant`） | asset |
| ASSET_DATABASE | — | `AssetDatabase` | asset |
| SUB_ASSET | `Sprite` | `SubAssetRecord` | asset |
| SCENE_ASSET | `Scene`（実行時ハンドル） | `SceneData`（ファイル表現） | serialization |
| PREFAB_ASSET / PREFAB_INSTANCE / *_OVERRIDE | `Prefab` | `PrefabData`, `PrefabInstanceData`, `PropertyOverride` | scene |
| GAME_OBJECT | `GameObject` | `ObjectRecord`（ファイル）/ `ObjectSlot`（実行時） | scene |
| COMPONENT / SERIALIZED_PROPERTY | `Component`, `Ref<T>` | `ComponentRecord`, `PropertyValue`（variant） | scene / serialization |
| 各コンポーネント（図 1-C） | `SpriteRenderer` 等（公開クラス、データは `m_`） | `ComponentAccess`（friend） | 各モジュール |
| ANIMATION_CLIP 〜 TRANSITION_CONDITION | `Animator`（パラメータ操作のみ） | `AnimationClipData`, `AnimatorGraph`, `AnimatorRuntime` | animation |
| TILESET / TILE_DEF / RULE_TILE | `Tilemap` | `TilesetData`, `RuleTileEvaluator` | render |
| PARTICLE_EFFECT / MODULE | `ParticleSystem` | `ParticleEffectData`, `ParticleSimulator` | render |
| INPUT_ACTION_* | `InputAction`, `Input` | `ActionMapRuntime`, `BindingResolver` | input |
| STRING_TABLE / LOCALE | `Localization` | `StringTableData` | persistence(core) |
| SAVE_SLOT / PLAYER_PREFS | `SaveSystem`, `PlayerPrefs` | `SaveFile` | persistence(core) |
| GAME_MODULE / BEHAVIOR_TYPE / PROPERTY_DESCRIPTOR | `SPROUT_BEHAVIOR`, `SPROUT_PROPERTY` | `ModuleRecord`, `TypeInfo`, `FieldInfo` | reflection |
| BUILD_PROFILE | — | `BuildProfile` | editor_core / cli |

### 1.2 ランタイム（図 2）
| ER | 型 | 備考 |
|---|---|---|
| ENGINE | `detail::Engine` | 公開側は `Game`（PImpl）|
| WORLD | `detail::World` | 公開側は `Scene` ハンドル |
| OBJECT_TABLE / OBJECT_SLOT | `detail::ObjectTable`, `detail::ObjectSlot` | SoA（名前・階層・状態を別配列）|
| COMPONENT_POOL / COMPONENT_SLOT | `detail::ComponentPool<T>`, `detail::IComponentPool` | 密配列 + sparse 索引 |
| PENDING_QUEUE | `detail::PendingQueue` | 反映時点は plan のフレーム順序に従う |
| TYPE_INFO / FIELD_INFO | `detail::TypeInfo`, `detail::FieldInfo` | |
| ASSET_SLOT | `detail::AssetSlot<T>` | 参照カウント・状態 |
| RENDER_QUEUE_ITEM / DRAW_BATCH | `detail::RenderItem`, `detail::DrawBatch` | |
| BODY_PROXY / CONTACT_RECORD | `detail::BodyProxy`, `detail::ContactEvent` | b2BodyId を内包 |
| VOICE | `detail::Voice` | |
| SUBSCRIPTION / TIMER / COROUTINE | `detail::Subscription`, `detail::TimerEntry`, `detail::CoroutineFrame` | コルーチンは C++20 coroutine（`sprout::Task`）|
| PROFILE_FRAME / SAMPLE | `detail::ProfileFrame`, `detail::ProfileSample` | |

### 1.3 エディタ（図 3）
`EditorSession`, `Selection`, `CommandHistory`, `EditorCommand`（派生: `CreateObjectCommand`, `DeleteObjectsCommand`, `SetPropertyCommand`, `ReparentCommand`, `AddComponentCommand`, `RemoveComponentCommand`, `PaintTilesCommand`, `ApplyPrefabOverrideCommand`, `CompositeCommand`）, `PlayModeController`, `WorldSnapshot`, `SceneDocument`, `AutosaveService`, `EditorLayout`, `EditorPreferences`, `BuildJob`, `CompileDiagnostic`。

## 2. 検証規則
| 対象 | 規則 | 違反時 |
|---|---|---|
| GameObject 親子 | 自分・子孫を親にできない | 拒否 + Warning |
| Prefab 入れ子 | 循環禁止（DAG 検査） | 作成/保存を拒否 |
| コンポーネント重複 | `TypeInfo::allowMultiple == false` の型は 1 つ | 既存を返し Warning |
| Transform | 削除不可 | 拒否 |
| レイヤー | 0–31 | 範囲外は 0 として Warning |
| Rigidbody2D | 同一オブジェクトに最大 1 | 同上 |
| 数値プロパティ | `Range` 属性内 | クランプ |
| アセット参照 | 型一致 | 代入拒否（エディタ）/ Missing 扱い（読込） |
| ファイル版 | `formatVersion <= current` | 新しい版は拒否、古い版は移行 |

## 3. 状態遷移

```text
ObjectSlot:     Free → Alive → PendingDestroy → Free(generation+1)
ComponentSlot:  Created → Awake → Enabled ⇄ Disabled → Started(初回 Update 前) → PendingDestroy → Destroyed
AssetSlot:      Unloaded → Loading → Loaded | Failed(→Fallback) ; Loaded → Reloading → Loaded ; refCount 0 → Unloaded
Engine:         Created → Running ⇄ Paused → Stopping → Stopped
PlayMode:       Edit → Playing ⇄ Paused → (Stepping) → Edit[snapshot 復元]
ModuleHost:     NoModule → Loaded → Reloading(snapshot→unload→load→restore) → Loaded | Failed(前版を保持)
BuildJob:       Queued → Compiling → Packing → Done | Failed
```

## 4. ライフサイクル呼出し順（Behavior）
`onCreate` → `onEnable` → `onStart`（初回更新直前）→ 毎フレーム `onFixedUpdate`×n → `onUpdate` → `onLateUpdate` → … → `onDisable` → `onDestroy`。衝突系は FixedUpdate の物理ステップ直後、UI イベントは Update 前、アニメイベントは Animation フェーズ。
