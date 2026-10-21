#include "platform/PlatformEvent.hpp"

#include <cstdint>
#include <gtest/gtest.h>
#include <optional>

namespace {

using sprout::detail::PlatformEvent;
using sprout::detail::PlatformEventType;
using sprout::detail::translatePlatformEvent;

std::optional<PlatformEvent> translate(const sf::Event& event)
{
    return translatePlatformEvent(event);
}

TEST(PlatformEventTests, Closed_TranslatesToClosed)
{
    const sf::Event event{sf::Event::Closed{}};

    const std::optional<PlatformEvent> translated = translate(event);

    ASSERT_TRUE(translated.has_value());
    EXPECT_EQ(translated->type, PlatformEventType::Closed);
}

TEST(PlatformEventTests, Resized_TranslatesToResizedWithSize)
{
    const sf::Event event{sf::Event::Resized{sf::Vector2u{1280U, 720U}}};

    const std::optional<PlatformEvent> translated = translate(event);

    ASSERT_TRUE(translated.has_value());
    EXPECT_EQ(translated->type, PlatformEventType::Resized);
    EXPECT_EQ(translated->size, (sprout::Vector2Int{1280, 720}));
}

TEST(PlatformEventTests, FocusGained_TranslatesToFocusGained)
{
    const sf::Event event{sf::Event::FocusGained{}};

    const std::optional<PlatformEvent> translated = translate(event);

    ASSERT_TRUE(translated.has_value());
    EXPECT_EQ(translated->type, PlatformEventType::FocusGained);
}

TEST(PlatformEventTests, FocusLost_TranslatesToFocusLost)
{
    const sf::Event event{sf::Event::FocusLost{}};

    const std::optional<PlatformEvent> translated = translate(event);

    ASSERT_TRUE(translated.has_value());
    EXPECT_EQ(translated->type, PlatformEventType::FocusLost);
}

TEST(PlatformEventTests, TextEntered_TranslatesToUnicodeCharacter)
{
    constexpr char32_t kHiraganaA = 0x3042;
    const sf::Event event{sf::Event::TextEntered{kHiraganaA}};

    const std::optional<PlatformEvent> translated = translate(event);

    ASSERT_TRUE(translated.has_value());
    EXPECT_EQ(translated->type, PlatformEventType::TextEntered);
    EXPECT_EQ(translated->character, kHiraganaA);
}

TEST(PlatformEventTests, TextEntered_TranslatesNonBmpCharacter)
{
    constexpr char32_t kEmoji = 0x1F600;
    const sf::Event event{sf::Event::TextEntered{kEmoji}};

    const std::optional<PlatformEvent> translated = translate(event);

    ASSERT_TRUE(translated.has_value());
    EXPECT_EQ(translated->type, PlatformEventType::TextEntered);
    EXPECT_EQ(translated->character, kEmoji);
}

TEST(PlatformEventTests, KeyPressed_TranslatesKeyScancodeAndModifiers)
{
    const sf::Event event{
        sf::Event::KeyPressed{sf::Keyboard::Key::A, sf::Keyboard::Scancode::A, false, true, false, false}};

    const std::optional<PlatformEvent> translated = translate(event);

    ASSERT_TRUE(translated.has_value());
    EXPECT_EQ(translated->type, PlatformEventType::KeyPressed);
    EXPECT_EQ(translated->key, sf::Keyboard::Key::A);
    EXPECT_EQ(translated->scancode, sf::Keyboard::Scancode::A);
    EXPECT_FALSE(translated->alt);
    EXPECT_TRUE(translated->control);
    EXPECT_FALSE(translated->shift);
    EXPECT_FALSE(translated->system);
}

TEST(PlatformEventTests, KeyReleased_TranslatesKeyAndScancode)
{
    const sf::Event event{
        sf::Event::KeyReleased{sf::Keyboard::Key::Space, sf::Keyboard::Scancode::Space, false, false, true, false}};

    const std::optional<PlatformEvent> translated = translate(event);

    ASSERT_TRUE(translated.has_value());
    EXPECT_EQ(translated->type, PlatformEventType::KeyReleased);
    EXPECT_EQ(translated->key, sf::Keyboard::Key::Space);
    EXPECT_EQ(translated->scancode, sf::Keyboard::Scancode::Space);
    EXPECT_TRUE(translated->shift);
}

TEST(PlatformEventTests, MouseButtonPressed_TranslatesButtonAndPosition)
{
    const sf::Event event{
        sf::Event::MouseButtonPressed{sf::Mouse::Button::Left, sf::Vector2i{12, 34}}};

    const std::optional<PlatformEvent> translated = translate(event);

    ASSERT_TRUE(translated.has_value());
    EXPECT_EQ(translated->type, PlatformEventType::MouseButtonPressed);
    EXPECT_EQ(translated->mouseButton, sf::Mouse::Button::Left);
    EXPECT_EQ(translated->position, (sprout::Vector2Int{12, 34}));
}

TEST(PlatformEventTests, MouseButtonReleased_TranslatesButtonAndPosition)
{
    const sf::Event event{
        sf::Event::MouseButtonReleased{sf::Mouse::Button::Right, sf::Vector2i{-5, 7}}};

    const std::optional<PlatformEvent> translated = translate(event);

    ASSERT_TRUE(translated.has_value());
    EXPECT_EQ(translated->type, PlatformEventType::MouseButtonReleased);
    EXPECT_EQ(translated->mouseButton, sf::Mouse::Button::Right);
    EXPECT_EQ(translated->position, (sprout::Vector2Int{-5, 7}));
}

TEST(PlatformEventTests, MouseMoved_TranslatesPosition)
{
    const sf::Event event{sf::Event::MouseMoved{sf::Vector2i{100, 200}}};

    const std::optional<PlatformEvent> translated = translate(event);

    ASSERT_TRUE(translated.has_value());
    EXPECT_EQ(translated->type, PlatformEventType::MouseMoved);
    EXPECT_EQ(translated->position, (sprout::Vector2Int{100, 200}));
}

TEST(PlatformEventTests, MouseMovedRaw_TranslatesDelta)
{
    const sf::Event event{sf::Event::MouseMovedRaw{sf::Vector2i{3, -4}}};

    const std::optional<PlatformEvent> translated = translate(event);

    ASSERT_TRUE(translated.has_value());
    EXPECT_EQ(translated->type, PlatformEventType::MouseMovedRaw);
    EXPECT_EQ(translated->moveDelta, (sprout::Vector2Int{3, -4}));
}

TEST(PlatformEventTests, MouseWheelScrolled_TranslatesWheelDeltaAndPosition)
{
    const sf::Event event{
        sf::Event::MouseWheelScrolled{sf::Mouse::Wheel::Horizontal, -1.5F, sf::Vector2i{8, 16}}};

    const std::optional<PlatformEvent> translated = translate(event);

    ASSERT_TRUE(translated.has_value());
    EXPECT_EQ(translated->type, PlatformEventType::MouseWheelScrolled);
    EXPECT_EQ(translated->wheel, sf::Mouse::Wheel::Horizontal);
    EXPECT_FLOAT_EQ(translated->wheelDelta, -1.5F);
    EXPECT_EQ(translated->position, (sprout::Vector2Int{8, 16}));
}

TEST(PlatformEventTests, UnsupportedEvent_ReturnsNullopt)
{
    const sf::Event joystick{sf::Event::JoystickConnected{0U}};
    const sf::Event joystickButton{sf::Event::JoystickButtonPressed{0U, 1U}};
    const sf::Event touch{sf::Event::TouchBegan{0U, sf::Vector2i{1, 2}}};
    const sf::Event sensor{sf::Event::SensorChanged{sf::Sensor::Type::Accelerometer, sf::Vector3f{1.0F, 2.0F, 3.0F}}};
    const sf::Event mouseEntered{sf::Event::MouseEntered{}};
    const sf::Event mouseLeft{sf::Event::MouseLeft{}};

    EXPECT_FALSE(translate(joystick).has_value());
    EXPECT_FALSE(translate(joystickButton).has_value());
    EXPECT_FALSE(translate(touch).has_value());
    EXPECT_FALSE(translate(sensor).has_value());
    EXPECT_FALSE(translate(mouseEntered).has_value());
    EXPECT_FALSE(translate(mouseLeft).has_value());
}

}  // namespace
