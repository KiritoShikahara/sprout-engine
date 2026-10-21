#pragma once

#include "sprout/math/Vector2.hpp"

#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

#include <optional>

namespace sprout::detail {

/// @brief SFML イベントを正規化した内部イベントの種類。
enum class PlatformEventType {
    Closed,
    Resized,
    FocusGained,
    FocusLost,
    TextEntered,
    KeyPressed,
    KeyReleased,
    MouseButtonPressed,
    MouseButtonReleased,
    MouseMoved,
    MouseMovedRaw,
    MouseWheelScrolled,
};

/// @brief SFML に依存しない形へ正規化したプラットフォームイベント。
///
/// 種類ごとに使うフィールドだけが設定される。座標はウィンドウ左上を原点とするピクセル。
struct PlatformEvent {
    /// イベントの種類。
    PlatformEventType type{PlatformEventType::Closed};

    /// マウス位置（ピクセル）。
    Vector2Int position{};

    /// ウィンドウの新しい大きさ（Resized のみ）。
    Vector2Int size{};

    /// 生のマウス移動量（MouseMovedRaw のみ）。
    Vector2Int moveDelta{};

    /// ホイールの回転量（MouseWheelScrolled のみ）。
    float wheelDelta{0.0F};

    /// 回転したホイール（MouseWheelScrolled のみ）。
    sf::Mouse::Wheel wheel{sf::Mouse::Wheel::Vertical};

    /// 対象のマウスボタン。
    sf::Mouse::Button mouseButton{sf::Mouse::Button::Left};

    /// 対象のキー。
    sf::Keyboard::Key key{sf::Keyboard::Key::Unknown};

    /// 対象キーの物理コード。
    sf::Keyboard::Scancode scancode{sf::Keyboard::Scancode::Unknown};

    /// 入力された文字（TextEntered のみ、UTF-32）。
    char32_t character{0};

    /// Alt キーが押されているか。
    bool alt{false};

    /// Control キーが押されているか。
    bool control{false};

    /// Shift キーが押されているか。
    bool shift{false};

    /// System キーが押されているか。
    bool system{false};
};

/// @brief SFML イベントを内部の PlatformEvent へ変換する。
/// @param event 変換する SFML イベント。
/// @return 対応する PlatformEvent。扱わないイベント（ジョイスティック・タッチ・センサー等）は std::nullopt。
[[nodiscard]] std::optional<PlatformEvent> translatePlatformEvent(const sf::Event& event);

}  // namespace sprout::detail
