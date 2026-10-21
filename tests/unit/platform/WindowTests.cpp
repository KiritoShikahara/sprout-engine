#include "platform/Window.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace {

using sprout::detail::Window;
using sprout::detail::WindowBackend;
using sprout::detail::WindowSettings;
using sprout::Error;
using sprout::ErrorCode;
using sprout::Result;
using sprout::Vector2Int;

/// @brief 呼び出しを記録し、状態を保持する擬似バックエンド。
class RecordingWindowBackend final : public WindowBackend {
public:
    Result<void> open(const WindowSettings& settings) override
    {
        ++openCount;

        if (openSucceeds) {
            opened = true;
            lastSettings = settings;
            m_title = settings.title;
            m_size = settings.size;
            m_fullscreen = settings.fullscreen;
            m_verticalSync = settings.verticalSync;
            return {};
        }
        return Error{ErrorCode::Unknown, "ウィンドウを作成できませんでした"};
    }

    void close() override
    {
        ++closeCount;
        opened = false;
    }

    [[nodiscard]] bool isOpen() const override
    {
        return opened;
    }

    void setTitle(std::string_view title) override
    {
        m_title.assign(title);
    }

    [[nodiscard]] std::string title() const override
    {
        return m_title;
    }

    void setSize(Vector2Int size) override
    {
        m_size = size;
    }

    [[nodiscard]] Vector2Int size() const override
    {
        return m_size;
    }

    void setFullscreen(bool enabled) override
    {
        m_fullscreen = enabled;
    }

    [[nodiscard]] bool isFullscreen() const override
    {
        return m_fullscreen;
    }

    void setVerticalSync(bool enabled) override
    {
        m_verticalSync = enabled;
    }

    [[nodiscard]] bool isVerticalSyncEnabled() const override
    {
        return m_verticalSync;
    }

    void setIcon(Vector2Int size, const std::uint8_t* pixels) override
    {
        iconSize = size;
        const std::size_t count = static_cast<std::size_t>(size.x) * static_cast<std::size_t>(size.y) * 4U;
        iconPixels.assign(pixels, pixels + count);
    }

    int openCount{0};
    int closeCount{0};
    bool openSucceeds{true};
    bool opened{false};
    WindowSettings lastSettings{};
    Vector2Int iconSize{};
    std::vector<std::uint8_t> iconPixels;

private:
    std::string m_title;
    Vector2Int m_size{};
    bool m_fullscreen{false};
    bool m_verticalSync{false};
};

TEST(WindowTests, Open_CreatesWindowWithGivenSettings)
{
    RecordingWindowBackend backend;
    Window window{backend};
    const WindowSettings settings{.title = "Sprout Test", .size = {800, 600}, .fullscreen = false,
        .verticalSync = true};

    const Result<void> result = window.open(settings);

    ASSERT_TRUE(result.isOk());
    EXPECT_TRUE(window.isOpen());
    EXPECT_EQ(backend.openCount, 1);
    EXPECT_EQ(backend.lastSettings.title, "Sprout Test");
    EXPECT_EQ(backend.lastSettings.size, (Vector2Int{800, 600}));
    EXPECT_FALSE(backend.lastSettings.fullscreen);
    EXPECT_TRUE(backend.lastSettings.verticalSync);
}

TEST(WindowTests, Open_InvalidSize_ReturnsInvalidArgument)
{
    RecordingWindowBackend backend;
    Window window{backend};

    const Result<void> result = window.open(WindowSettings{.size = {0, 600}});

    EXPECT_FALSE(result.isOk());
    EXPECT_EQ(result.error().code(), ErrorCode::InvalidArgument);
    EXPECT_EQ(backend.openCount, 0);
    EXPECT_FALSE(window.isOpen());
}

TEST(WindowTests, Open_BackendFailure_ReturnsError)
{
    RecordingWindowBackend backend;
    backend.openSucceeds = false;
    Window window{backend};

    const Result<void> result = window.open(WindowSettings{});

    EXPECT_FALSE(result.isOk());
    EXPECT_EQ(backend.openCount, 1);
    EXPECT_FALSE(window.isOpen());
}

TEST(WindowTests, Close_ClosesWindow)
{
    RecordingWindowBackend backend;
    Window window{backend};
    ASSERT_TRUE(window.open(WindowSettings{}).isOk());

    window.close();

    EXPECT_FALSE(window.isOpen());
    EXPECT_EQ(backend.closeCount, 1);
}

TEST(WindowTests, Destructor_ClosesWindow)
{
    RecordingWindowBackend backend;

    {
        Window window{backend};
        ASSERT_TRUE(window.open(WindowSettings{}).isOk());
    }

    EXPECT_EQ(backend.closeCount, 1);
    EXPECT_FALSE(backend.opened);
}

TEST(WindowTests, SetSize_ChangesResolution)
{
    RecordingWindowBackend backend;
    Window window{backend};
    const Result<void> openResult = window.open(WindowSettings{.size = {800, 600}});
    ASSERT_TRUE(openResult.isOk());

    const Result<void> result = window.setSize(Vector2Int{1920, 1080});

    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(window.size(), (Vector2Int{1920, 1080}));
}

TEST(WindowTests, SetSize_InvalidSize_ReturnsInvalidArgument)
{
    RecordingWindowBackend backend;
    Window window{backend};
    const Result<void> openResult = window.open(WindowSettings{.size = {800, 600}});
    ASSERT_TRUE(openResult.isOk());

    const Result<void> result = window.setSize(Vector2Int{1280, -1});

    EXPECT_FALSE(result.isOk());
    EXPECT_EQ(result.error().code(), ErrorCode::InvalidArgument);
    EXPECT_EQ(window.size(), (Vector2Int{800, 600}));
}

TEST(WindowTests, SetSize_WhileClosed_ReturnsInvalidState)
{
    RecordingWindowBackend backend;
    Window window{backend};

    const Result<void> result = window.setSize(Vector2Int{1280, 720});

    EXPECT_FALSE(result.isOk());
    EXPECT_EQ(result.error().code(), ErrorCode::InvalidState);
}

TEST(WindowTests, SetFullscreen_TogglesFullscreenState)
{
    RecordingWindowBackend backend;
    Window window{backend};
    ASSERT_TRUE(window.open(WindowSettings{}).isOk());
    EXPECT_FALSE(window.isFullscreen());

    ASSERT_TRUE(window.setFullscreen(true).isOk());
    EXPECT_TRUE(window.isFullscreen());

    ASSERT_TRUE(window.setFullscreen(false).isOk());
    EXPECT_FALSE(window.isFullscreen());
}

TEST(WindowTests, SetFullscreen_WhileClosed_ReturnsInvalidState)
{
    RecordingWindowBackend backend;
    Window window{backend};

    const Result<void> result = window.setFullscreen(true);

    EXPECT_FALSE(result.isOk());
    EXPECT_EQ(result.error().code(), ErrorCode::InvalidState);
    EXPECT_FALSE(window.isFullscreen());
}

TEST(WindowTests, SetVerticalSync_TogglesVerticalSync)
{
    RecordingWindowBackend backend;
    Window window{backend};
    const Result<void> openResult = window.open(WindowSettings{.verticalSync = true});
    ASSERT_TRUE(openResult.isOk());
    EXPECT_TRUE(window.isVerticalSyncEnabled());

    ASSERT_TRUE(window.setVerticalSync(false).isOk());
    EXPECT_FALSE(window.isVerticalSyncEnabled());

    ASSERT_TRUE(window.setVerticalSync(true).isOk());
    EXPECT_TRUE(window.isVerticalSyncEnabled());
}

TEST(WindowTests, SetVerticalSync_WhileClosed_ReturnsInvalidState)
{
    RecordingWindowBackend backend;
    Window window{backend};

    const Result<void> result = window.setVerticalSync(true);

    EXPECT_FALSE(result.isOk());
    EXPECT_EQ(result.error().code(), ErrorCode::InvalidState);
}

TEST(WindowTests, SetTitle_UpdatesTitle)
{
    RecordingWindowBackend backend;
    Window window{backend};
    const Result<void> openResult = window.open(WindowSettings{.title = "Before"});
    ASSERT_TRUE(openResult.isOk());

    const Result<void> result = window.setTitle("After");

    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(window.title(), "After");
}

TEST(WindowTests, SetTitle_WhileClosed_ReturnsInvalidState)
{
    RecordingWindowBackend backend;
    Window window{backend};

    const Result<void> result = window.setTitle("After");

    EXPECT_FALSE(result.isOk());
    EXPECT_EQ(result.error().code(), ErrorCode::InvalidState);
}

TEST(WindowTests, SetIcon_AppliesIconPixels)
{
    RecordingWindowBackend backend;
    Window window{backend};
    ASSERT_TRUE(window.open(WindowSettings{}).isOk());
    std::vector<std::uint8_t> pixels(16U * 16U * 4U, 0x7F);
    pixels[0] = 0xFF;

    const Result<void> result = window.setIcon(Vector2Int{16, 16}, pixels.data());

    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(backend.iconSize, (Vector2Int{16, 16}));
    ASSERT_EQ(backend.iconPixels.size(), pixels.size());
    EXPECT_EQ(backend.iconPixels.front(), 0xFF);
}

TEST(WindowTests, SetIcon_NullPixels_ReturnsInvalidArgument)
{
    RecordingWindowBackend backend;
    Window window{backend};
    ASSERT_TRUE(window.open(WindowSettings{}).isOk());

    const Result<void> result = window.setIcon(Vector2Int{16, 16}, nullptr);

    EXPECT_FALSE(result.isOk());
    EXPECT_EQ(result.error().code(), ErrorCode::InvalidArgument);
}

TEST(WindowTests, SetIcon_InvalidSize_ReturnsInvalidArgument)
{
    RecordingWindowBackend backend;
    Window window{backend};
    ASSERT_TRUE(window.open(WindowSettings{}).isOk());
    std::vector<std::uint8_t> pixels(4U, 0x00);

    const Result<void> result = window.setIcon(Vector2Int{0, 16}, pixels.data());

    EXPECT_FALSE(result.isOk());
    EXPECT_EQ(result.error().code(), ErrorCode::InvalidArgument);
}

TEST(WindowTests, SetIcon_WhileClosed_ReturnsInvalidState)
{
    RecordingWindowBackend backend;
    Window window{backend};
    std::vector<std::uint8_t> pixels(4U, 0x00);

    const Result<void> result = window.setIcon(Vector2Int{2, 2}, pixels.data());

    EXPECT_FALSE(result.isOk());
    EXPECT_EQ(result.error().code(), ErrorCode::InvalidState);
}

}  // namespace
