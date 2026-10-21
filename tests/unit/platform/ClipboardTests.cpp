#include "platform/Clipboard.hpp"

#include <gtest/gtest.h>

#include <string>
#include <string_view>

namespace {

using sprout::detail::Clipboard;
using sprout::detail::ClipboardBackend;

class MemoryClipboardBackend final : public ClipboardBackend {
public:
    [[nodiscard]] std::string read() const override
    {
        return m_text;
    }

    void write(std::string_view text) override
    {
        m_text.assign(text);
    }

private:
    std::string m_text;
};

TEST(ClipboardTests, Text_InitiallyEmpty)
{
    MemoryClipboardBackend backend;
    const Clipboard clipboard{backend};

    EXPECT_EQ(clipboard.text(), "");
    EXPECT_FALSE(clipboard.hasText());
}

TEST(ClipboardTests, SetText_StoresUtf8Text)
{
    MemoryClipboardBackend backend;
    Clipboard clipboard{backend};

    clipboard.setText("こんにちは、Sprout!");

    EXPECT_EQ(clipboard.text(), "こんにちは、Sprout!");
    EXPECT_TRUE(clipboard.hasText());
}

TEST(ClipboardTests, SetText_OverwritesPreviousText)
{
    MemoryClipboardBackend backend;
    Clipboard clipboard{backend};

    clipboard.setText("first");
    clipboard.setText("second");

    EXPECT_EQ(clipboard.text(), "second");
}

TEST(ClipboardTests, Text_RoundTripsMultilineText)
{
    MemoryClipboardBackend backend;
    Clipboard clipboard{backend};
    const std::string original = "line 1\nline 2\r\n日本語\t終わり";

    clipboard.setText(original);

    EXPECT_EQ(clipboard.text(), original);
}

TEST(ClipboardTests, SetText_Empty_ClearsText)
{
    MemoryClipboardBackend backend;
    Clipboard clipboard{backend};
    clipboard.setText("temporary");

    clipboard.setText("");

    EXPECT_EQ(clipboard.text(), "");
    EXPECT_FALSE(clipboard.hasText());
}

TEST(ClipboardTests, Clear_RemovesText)
{
    MemoryClipboardBackend backend;
    Clipboard clipboard{backend};
    clipboard.setText("temporary");

    clipboard.clear();

    EXPECT_EQ(clipboard.text(), "");
    EXPECT_FALSE(clipboard.hasText());
}

TEST(ClipboardTests, Instances_UseTheirOwnBackend)
{
    MemoryClipboardBackend first;
    MemoryClipboardBackend second;
    Clipboard firstClipboard{first};
    Clipboard secondClipboard{second};

    firstClipboard.setText("first");
    secondClipboard.setText("second");

    EXPECT_EQ(firstClipboard.text(), "first");
    EXPECT_EQ(secondClipboard.text(), "second");
}

}  // namespace
