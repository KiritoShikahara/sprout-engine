# Contract: 公開 API（engine/include/sprout）

利用者（学生）が使えるのは本契約の型のみ。全て `namespace sprout`。外部ライブラリ型・`detail` 型は現れない。例外は投げない。

## 1. 参照モデル
```cpp
class GameObject {               // 16 byte 値型ハンドル
public:
    bool isValid() const;  explicit operator bool() const;
    std::string_view name() const;            void setName(std::string_view name);
    bool isActiveSelf() const;  bool isActiveInHierarchy() const;  void setActive(bool isActive);
    std::string_view tag() const;  void setTag(std::string_view tag);  bool hasTag(std::string_view tag) const;
    int layer() const;  void setLayer(int layer);
    Transform& transform();                   // 無効時は共有ダミー（書込は無視 + Error ログ）

    template <typename TComponent> Ref<TComponent> addComponent();
    template <typename TComponent> Ref<TComponent> component() const;          // 無ければ無効 Ref
    template <typename TComponent> Ref<TComponent> componentInChildren() const;
    template <typename TComponent> Ref<TComponent> componentInParent() const;
    template <typename TComponent> std::vector<Ref<TComponent>> components() const;
    template <typename TComponent> bool hasComponent() const;
    template <typename TComponent> void removeComponent();

    void setParent(GameObject parent, bool keepWorldPosition = true);
    GameObject parent() const;  std::span<const GameObject> children() const;
    void destroy(float delaySeconds = 0.f);
    Scene scene() const;
};

template <typename TComponent>
class Ref {                       // コンポーネントへの無効化検出付き参照
public:
    bool isValid() const;  explicit operator bool() const;
    TComponent* operator->() const;            // 毎回解決。無効時はダミー + Error ログ
    TComponent& operator*() const;
    GameObject gameObject() const;
};
```
テンプレートは `TypeId<T>()`（登録済み型 ID）と非テンプレートの輸出関数 `addComponentById(TypeId)` 等へ委譲し、格納構造をヘッダに出さない。

## 2. 振る舞い
```cpp
class Behavior : public Component {
protected:                         // ライフサイクル（全て任意実装）
    virtual void onCreate() {}  virtual void onEnable() {}  virtual void onStart() {}
    virtual void onFixedUpdate(float fixedDeltaTime) {}  virtual void onUpdate(float deltaTime) {}
    virtual void onLateUpdate(float deltaTime) {}  virtual void onDisable() {}  virtual void onDestroy() {}
    virtual void onCollisionEnter(const Collision& collision) {}  /* Stay / Exit */
    virtual void onTriggerEnter(GameObject other) {}              /* Stay / Exit */
    virtual void onAnimationEvent(std::string_view eventName, const PropertyValue& argument) {}

    // コンテキスト（グローバル関数の代わり）
    GameObject gameObject() const;  Transform& transform() const;  Scene scene() const;
    Input& input() const;  Time& time() const;  Audio& audio() const;  Assets& assets() const;
    SceneManager& sceneManager() const;  Physics2D& physics() const;  DebugDraw& debugDraw() const;

    GameObject instantiate(const AssetRef<Prefab>& prefab, Vector2 position, GameObject parent = {});
    TimerHandle after(float seconds, std::function<void()> callback);
    TimerHandle every(float seconds, std::function<void()> callback);
    CoroutineHandle startCoroutine(Task task);   // co_await waitSeconds(1.f) / nextFrame() / waitUntil(pred)
};

#define SPROUT_BEHAVIOR(TypeName)               // 型登録（クラス定義の後に 1 行）
#define SPROUT_PROPERTY(...)                    // 直後のメンバを公開: Range(0,500), Tooltip("…"), Header("…")
#define SPROUT_GAME_MODULE(ModuleName)          // ゲームモジュール入口（テンプレートに記載済み）
```

## 3. サブシステム契約（抽粋）
| 型 | 主要メンバ |
|---|---|
| `Input` | `action(name) -> InputAction` (`isPressed/isHeld/isReleased/value<float>/value<Vector2>`), `isKeyHeld(Key)`, `mouseScreenPosition()`, `mouseWorldPosition(Camera)`, `gamepadCount()`, `rebind(...)` |
| `Time` | `deltaTime()`, `unscaledDeltaTime()`, `fixedDeltaTime()`, `timeScale()/setTimeScale()`, `elapsed()`, `frameCount()` |
| `Audio` | `playOneShot(AssetRef<AudioClip>, float volume = 1.f, std::string_view group = "SE")`, `playMusic(clip, float fadeSeconds)`, `stopMusic(float fadeSeconds)`, `setGroupVolume(group, float)` |
| `Assets` | `load<T>(path) -> AssetRef<T>`, `loadAsync<T>(path) -> AsyncOperation<T>`, `unloadUnused()` |
| `SceneManager` | `load(name, LoadMode::Single/Additive, Transition)`, `loadAsync(...)`, `unload(name)`, `activeScene()`, `markPersistent(GameObject)` |
| `Physics2D` | `raycast(origin, direction, distance, LayerMask) -> std::optional<RaycastHit>`, `overlapCircle/Box(...)`, `gravity()/setGravity()` |
| `Scene` | `createObject(name)`, `find(name)`, `findWithTag(tag)`, `findAll<T>()`, `name()` |
| `Transform` | `position()/setPosition()`, `localPosition()`, `rotation()`, `scale()`, `translate(Vector2)`, `rotate(float)`, `lookAt(Vector2)`, `worldToLocal/localToWorld` |
| `Rigidbody2D` | `velocity()/setVelocity()`, `addForce(Vector2, ForceMode)`, `addTorque`, `bodyType/setBodyType`, `gravityScale` |
| `Animator` | `setFloat/setInt/setBool(name, v)`, `setTrigger(name)`, `play(stateName)`, `currentState()` |
| `Tween` | `Tween::move(transform, to, seconds).ease(Ease::OutQuad).delay(0.2f).loop(LoopMode::YoYo)`、`Sequence` / `Parallel` |
| `SaveSystem` | `save(slot, const SaveData&) -> Result<void>`, `load(slot) -> Result<SaveData>`, `exists(slot)`、`SaveData` はキー値木（int/float/bool/string/array/object） |
| `Localization` | `text(key) -> std::string_view`, `setLocale(code)`, `onLocaleChanged` イベント |
| `DebugDraw` | `line/rect/circle/text(..., Color, float durationSeconds = 0)`（書き出し版では空実装） |
| `Log` | `Log::info(fmt, args...)`, `warning`, `error`（`std::format` 書式） |
| `Result<T>` | `isOk()`, `value()`, `error().message()`, `error().code()` |

組込みコンポーネントの全プロパティは ER 図 1-C の属性と 1:1（getter = 名詞、setter = `set` + 名詞）。

## 4. 例（SC-001 基準）
```cpp
#include <sprout/Sprout.hpp>

class PlayerMover final : public sprout::Behavior {
    SPROUT_PROPERTY(Range(0, 1000), Tooltip("秒速（ピクセル）"))
    float m_speed = 200.f;

    void onUpdate(float deltaTime) override {
        const sprout::Vector2 move = input().action("Move").value<sprout::Vector2>();
        transform().translate(move * m_speed * deltaTime);
        if (input().action("Jump").isPressed()) {
            audio().playOneShot(assets().load<sprout::AudioClip>("Audio/jump.wav"));
        }
    }
};
SPROUT_BEHAVIOR(PlayerMover)
```
エディタを使わない場合のコードのみ起動は `sprout::Game` を使う（quickstart 参照）。

## 5. 互換性
- 公開 API は semver。MINOR で追加のみ、削除は `[[deprecated("…を使ってください")]]` を 1 MINOR 経て MAJOR で。
