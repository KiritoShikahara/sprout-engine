#include "platform/Window.hpp"

#include <SFML/System/String.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/Window.hpp>

#include <cstddef>
#include <string>
#include <vector>

namespace {

constexpr std::size_t kBytesPerPixel = 4;

sf::String toSfString(std::string_view text)
{
    return sf::String::fromUtf8(text.begin(), text.end());
}

sf::Vector2u toSfSize(const sprout::Vector2Int& size) noexcept
{
    return sf::Vector2u{static_cast<unsigned int>(size.x), static_cast<unsigned int>(size.y)};
}

class SfmlWindowBackend final : public sprout::detail::WindowBackend {
public:
    sprout::Result<void> open(const sprout::detail::WindowSettings& settings) override
    {
        m_settings = settings;
        create();

        if (!m_window.isOpen()) {
            return sprout::Error{sprout::ErrorCode::Unknown, "ウィンドウを作成できませんでした"};
        }
        return {};
    }

    void close() override
    {
        m_window.close();
    }

    [[nodiscard]] bool isOpen() const override
    {
        return m_window.isOpen();
    }

    void setTitle(std::string_view title) override
    {
        m_settings.title.assign(title);
        m_window.setTitle(toSfString(title));
    }

    [[nodiscard]] std::string title() const override
    {
        return m_settings.title;
    }

    void setSize(sprout::Vector2Int size) override
    {
        m_settings.size = size;
        m_window.setSize(toSfSize(size));
    }

    [[nodiscard]] sprout::Vector2Int size() const override
    {
        const sf::Vector2u current = m_window.getSize();
        return sprout::Vector2Int{static_cast<int>(current.x), static_cast<int>(current.y)};
    }

    void setFullscreen(bool enabled) override
    {
        if (m_settings.fullscreen == enabled) {
            return;
        }

        m_settings.fullscreen = enabled;
        if (m_window.isOpen()) {
            create();
        }
    }

    [[nodiscard]] bool isFullscreen() const override
    {
        return m_settings.fullscreen;
    }

    void setVerticalSync(bool enabled) override
    {
        m_settings.verticalSync = enabled;
        m_window.setVerticalSyncEnabled(enabled);
    }

    [[nodiscard]] bool isVerticalSyncEnabled() const override
    {
        return m_settings.verticalSync;
    }

    void setIcon(sprout::Vector2Int size, const std::uint8_t* pixels) override
    {
        m_iconSize = size;
        m_iconPixels.assign(pixels, pixels + pixelCount(size));
        m_window.setIcon(toSfSize(size), pixels);
    }

private:
    static std::size_t pixelCount(const sprout::Vector2Int& size) noexcept
    {
        return static_cast<std::size_t>(size.x) * static_cast<std::size_t>(size.y) * kBytesPerPixel;
    }

    /// 現在の設定でウィンドウを作り直す（フルスクリーン切替時に使う）。
    void create()
    {
        const sf::State state = m_settings.fullscreen ? sf::State::Fullscreen : sf::State::Windowed;
        m_window.create(sf::VideoMode{toSfSize(m_settings.size)}, toSfString(m_settings.title),
            static_cast<std::uint32_t>(sf::Style::Default), state);
        m_window.setVerticalSyncEnabled(m_settings.verticalSync);

        if (!m_iconPixels.empty()) {
            m_window.setIcon(toSfSize(m_iconSize), m_iconPixels.data());
        }
    }

    sf::Window m_window;
    sprout::detail::WindowSettings m_settings;
    sprout::Vector2Int m_iconSize{};
    std::vector<std::uint8_t> m_iconPixels;
};

}  // namespace

namespace sprout::detail {

WindowBackend& systemWindowBackend()
{
    static SfmlWindowBackend backend;
    return backend;
}

Window::Window()
    : m_backend{&systemWindowBackend()}
{
}

Window::Window(WindowBackend& backend) noexcept
    : m_backend{&backend}
{
}

Window::~Window()
{
    close();
}

Result<void> Window::open(const WindowSettings& settings)
{
    if (settings.size.x <= 0 || settings.size.y <= 0) {
        return Error{ErrorCode::InvalidArgument, "ウィンドウの大きさは 1 ピクセル以上で指定してください"};
    }

    return m_backend->open(settings);
}

void Window::close()
{
    m_backend->close();
}

bool Window::isOpen() const
{
    return m_backend->isOpen();
}

Result<void> Window::setTitle(std::string_view title)
{
    if (!isOpen()) {
        return Error{ErrorCode::InvalidState, "ウィンドウが閉じているためタイトルを変更できません"};
    }

    m_backend->setTitle(title);
    return {};
}

std::string Window::title() const
{
    return m_backend->title();
}

Result<void> Window::setSize(Vector2Int size)
{
    if (size.x <= 0 || size.y <= 0) {
        return Error{ErrorCode::InvalidArgument, "ウィンドウの大きさは 1 ピクセル以上で指定してください"};
    }
    if (!isOpen()) {
        return Error{ErrorCode::InvalidState, "ウィンドウが閉じているため解像度を変更できません"};
    }

    m_backend->setSize(size);
    return {};
}

Vector2Int Window::size() const
{
    return m_backend->size();
}

Result<void> Window::setFullscreen(bool enabled)
{
    if (!isOpen()) {
        return Error{ErrorCode::InvalidState, "ウィンドウが閉じているため表示モードを変更できません"};
    }

    m_backend->setFullscreen(enabled);
    return {};
}

bool Window::isFullscreen() const
{
    return m_backend->isFullscreen();
}

Result<void> Window::setVerticalSync(bool enabled)
{
    if (!isOpen()) {
        return Error{ErrorCode::InvalidState, "ウィンドウが閉じているため垂直同期を変更できません"};
    }

    m_backend->setVerticalSync(enabled);
    return {};
}

bool Window::isVerticalSyncEnabled() const
{
    return m_backend->isVerticalSyncEnabled();
}

Result<void> Window::setIcon(Vector2Int size, const std::uint8_t* pixels)
{
    if (size.x <= 0 || size.y <= 0) {
        return Error{ErrorCode::InvalidArgument, "アイコンの大きさは 1 ピクセル以上で指定してください"};
    }
    if (pixels == nullptr) {
        return Error{ErrorCode::InvalidArgument, "アイコンのピクセルデータがありません"};
    }
    if (!isOpen()) {
        return Error{ErrorCode::InvalidState, "ウィンドウが閉じているためアイコンを設定できません"};
    }

    m_backend->setIcon(size, pixels);
    return {};
}

}  // namespace sprout::detail
