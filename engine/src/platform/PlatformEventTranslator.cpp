#include "platform/PlatformEvent.hpp"

namespace sprout::detail {

namespace {

Vector2Int toPixelPosition(const sf::Vector2i& value) noexcept
{
    return Vector2Int{value.x, value.y};
}

Vector2Int toPixelSize(const sf::Vector2u& value) noexcept
{
    return Vector2Int{static_cast<int>(value.x), static_cast<int>(value.y)};
}

template <typename TKeyEvent>
void copyKeyState(PlatformEvent& event, const TKeyEvent& source) noexcept
{
    event.key = source.code;
    event.scancode = source.scancode;
    event.alt = source.alt;
    event.control = source.control;
    event.shift = source.shift;
    event.system = source.system;
}

}  // namespace

std::optional<PlatformEvent> translatePlatformEvent(const sf::Event& event)
{
    PlatformEvent translated;

    if (event.is<sf::Event::Closed>()) {
        translated.type = PlatformEventType::Closed;
        return translated;
    }
    if (const auto* resized = event.getIf<sf::Event::Resized>()) {
        translated.type = PlatformEventType::Resized;
        translated.size = toPixelSize(resized->size);
        return translated;
    }
    if (event.is<sf::Event::FocusGained>()) {
        translated.type = PlatformEventType::FocusGained;
        return translated;
    }
    if (event.is<sf::Event::FocusLost>()) {
        translated.type = PlatformEventType::FocusLost;
        return translated;
    }
    if (const auto* text = event.getIf<sf::Event::TextEntered>()) {
        translated.type = PlatformEventType::TextEntered;
        translated.character = text->unicode;
        return translated;
    }
    if (const auto* pressed = event.getIf<sf::Event::KeyPressed>()) {
        translated.type = PlatformEventType::KeyPressed;
        copyKeyState(translated, *pressed);
        return translated;
    }
    if (const auto* released = event.getIf<sf::Event::KeyReleased>()) {
        translated.type = PlatformEventType::KeyReleased;
        copyKeyState(translated, *released);
        return translated;
    }
    if (const auto* button = event.getIf<sf::Event::MouseButtonPressed>()) {
        translated.type = PlatformEventType::MouseButtonPressed;
        translated.mouseButton = button->button;
        translated.position = toPixelPosition(button->position);
        return translated;
    }
    if (const auto* button = event.getIf<sf::Event::MouseButtonReleased>()) {
        translated.type = PlatformEventType::MouseButtonReleased;
        translated.mouseButton = button->button;
        translated.position = toPixelPosition(button->position);
        return translated;
    }
    if (const auto* moved = event.getIf<sf::Event::MouseMoved>()) {
        translated.type = PlatformEventType::MouseMoved;
        translated.position = toPixelPosition(moved->position);
        return translated;
    }
    if (const auto* raw = event.getIf<sf::Event::MouseMovedRaw>()) {
        translated.type = PlatformEventType::MouseMovedRaw;
        translated.moveDelta = toPixelPosition(raw->delta);
        return translated;
    }
    if (const auto* wheel = event.getIf<sf::Event::MouseWheelScrolled>()) {
        translated.type = PlatformEventType::MouseWheelScrolled;
        translated.wheel = wheel->wheel;
        translated.wheelDelta = wheel->delta;
        translated.position = toPixelPosition(wheel->position);
        return translated;
    }

    return std::nullopt;
}

}  // namespace sprout::detail
