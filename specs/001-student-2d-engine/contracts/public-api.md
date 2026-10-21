# Contract: 公開 API（include/sprout/）

利用者が使ってよいのは以下のみ。全て `namespace sprout`。外部ライブラリの型は現れない。

```cpp
// core/Game.hpp
class Game final {
public:
    explicit Game(GameConfig config);
    ~Game();                                   // PImpl
    Scene& createScene(std::string_view name);
    void loadScene(std::string_view nameOrPath); // フレーム末に切替
    int run();                                 // 終了コードを返す
    void quit();
};

// scene/Scene.hpp
class Scene {
public:
    GameObject createObject(std::string_view name);
    GameObject findObject(std::string_view name) const; // 無ければ無効ハンドル
    void destroyObject(GameObject object);
    Result<void> saveToFile(std::string_view path) const;
};

// scene/GameObject.hpp  （値型ハンドル。コピー可）
class GameObject {
public:
    bool isValid() const;
    std::string_view name() const;
    bool isActive() const;  void setActive(bool active);
    Transform& transform();
    template <typename TComponent> TComponent& addComponent();
    template <typename TComponent> TComponent* findComponent();   // 無ければ nullptr（所有しない）
    template <typename TComponent> bool hasComponent() const;
    template <typename TComponent> void removeComponent();
    void setParent(GameObject parent);
    explicit operator bool() const { return isValid(); }
};

// component/Behavior.hpp  （利用者が継承する唯一の基底）
class Behavior : public Component {
public:
    virtual void onStart() {}
    virtual void onUpdate(float deltaTime) {}
    virtual void onCollisionEnter(GameObject other) {}
    virtual void onCollisionExit(GameObject other) {}
    virtual void onDestroy() {}
    virtual void declareProperties(PropertyList& properties) {}
protected:
    GameObject gameObject() const;
    Scene& scene() const;
};

// input/Input.hpp  （static 関数のみ）
namespace Input {
    bool isKeyDown(Key key);  bool isKeyPressed(Key key);  bool isKeyReleased(Key key);
    bool isMouseDown(MouseButton button);  Vector2 mousePosition();
}

// audio/Audio.hpp
namespace Audio { void playSound(std::string_view path, float volume = 100.f); void playMusic(std::string_view path, bool isLooping = true); void stopMusic(); }

// reflect/BehaviorRegistry.hpp
#define SPROUT_REGISTER_BEHAVIOR(TBehavior)   // エディタ登録用（唯一の公開マクロ）
```

## 例（SC-001 の基準）

```cpp
#include <sprout/Sprout.hpp>

class PlayerMover final : public sprout::Behavior {
public:
    void onUpdate(float deltaTime) override {
        sprout::Vector2 direction{};
        if (sprout::Input::isKeyDown(sprout::Key::Left))  direction.x -= 1.f;
        if (sprout::Input::isKeyDown(sprout::Key::Right)) direction.x += 1.f;
        gameObject().transform().move(direction * m_speed * deltaTime);
    }
    void declareProperties(sprout::PropertyList& properties) override { properties.add("speed", m_speed); }
private:
    float m_speed = 200.f;
};
SPROUT_REGISTER_BEHAVIOR(PlayerMover)

int main() {
    sprout::Game game({ .title = "Hello", .width = 800, .height = 600 });
    sprout::Scene& scene = game.createScene("Main");
    sprout::GameObject player = scene.createObject("Player");
    player.addComponent<sprout::SpriteRenderer>().setTexture("assets/player.png");
    player.addComponent<PlayerMover>();
    return game.run();
}
```

## エラー規約
- 例外は投げない。失敗は `Result<T>`（`isOk()`, `value()`, `error().message()`）または無効ハンドル + `Log` への日本語メッセージ。
