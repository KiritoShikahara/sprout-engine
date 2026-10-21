#include "core/Assert.hpp"
#include "core/ErrorCatalog.hpp"

#include <array>
#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_set>

#include <gtest/gtest.h>

namespace {

using sprout::ErrorCode;
using sprout::ErrorInfo;

constexpr std::array<ErrorCode, 12> kAllCodes{
    ErrorCode::Unknown,
    ErrorCode::InvalidArgument,
    ErrorCode::InvalidState,
    ErrorCode::NotFound,
    ErrorCode::AlreadyExists,
    ErrorCode::PermissionDenied,
    ErrorCode::OutOfRange,
    ErrorCode::NotSupported,
    ErrorCode::Io,
    ErrorCode::Parse,
    ErrorCode::OutOfMemory,
    ErrorCode::Timeout,
};

bool hasJapanese(std::string_view text)
{
    for (const char character : text) {
        if (static_cast<unsigned char>(character) >= 0x80U) {
            return true;
        }
    }
    return false;
}

struct CapturedAssert {
    bool called{false};
    std::string expression;
    std::string file;
    int line{0};
    std::string message;
    ErrorCode code{ErrorCode::Unknown};
};

CapturedAssert gCaptured;

void captureAssert(const sprout::AssertFailure& failure)
{
    gCaptured.called = true;
    gCaptured.expression = failure.expression == nullptr ? "" : failure.expression;
    gCaptured.file = failure.file == nullptr ? "" : failure.file;
    gCaptured.line = failure.line;
    gCaptured.message = std::string{failure.message};
    gCaptured.code = failure.code;
}

class ScopedAssertHandler {
public:
    explicit ScopedAssertHandler(sprout::AssertHandler handler)
    {
        sprout::setAssertHandler(handler);
    }

    ~ScopedAssertHandler()
    {
        sprout::setAssertHandler(nullptr);
    }

    ScopedAssertHandler(const ScopedAssertHandler&) = delete;
    ScopedAssertHandler& operator=(const ScopedAssertHandler&) = delete;
};

}  // namespace

TEST(ErrorCatalogTests, AllCodes_HaveJapaneseWhatWhereAndHowToFix)
{
    for (const ErrorCode code : kAllCodes) {
        SCOPED_TRACE(std::string{sprout::errorCodeName(code)});
        EXPECT_TRUE(sprout::isKnownErrorCode(code));

        const ErrorInfo& info = sprout::errorInfo(code);
        EXPECT_EQ(info.code, code);
        EXPECT_FALSE(info.what.empty());
        EXPECT_FALSE(info.where.empty());
        EXPECT_FALSE(info.howToFix.empty());
        EXPECT_TRUE(hasJapanese(info.what));
        EXPECT_TRUE(hasJapanese(info.where));
        EXPECT_TRUE(hasJapanese(info.howToFix));
    }
}

TEST(ErrorCatalogTests, AllCodes_HaveUniqueNames)
{
    std::unordered_set<std::string> names;
    for (const ErrorCode code : kAllCodes) {
        const std::string name{sprout::errorCodeName(code)};
        EXPECT_FALSE(name.empty());
        EXPECT_EQ(name.rfind("E_", 0), 0U);
        EXPECT_TRUE(names.insert(name).second) << name;
    }
}

TEST(ErrorCatalogTests, SpecificCodes_DescribeTheProblemInJapanese)
{
    EXPECT_NE(sprout::errorInfo(ErrorCode::NotFound).what.find("見つかりません"), std::string_view::npos);
    EXPECT_NE(sprout::errorInfo(ErrorCode::Io).what.find("失敗"), std::string_view::npos);
    EXPECT_NE(sprout::errorInfo(ErrorCode::Parse).what.find("解釈"), std::string_view::npos);
    EXPECT_NE(sprout::errorInfo(ErrorCode::OutOfRange).what.find("範囲"), std::string_view::npos);
    EXPECT_NE(sprout::errorInfo(ErrorCode::NotFound).howToFix.find("確認"), std::string_view::npos);
}

TEST(ErrorCatalogTests, UnknownCode_FallsBackToUnknownEntry)
{
    const ErrorInfo& info = sprout::errorInfo(static_cast<ErrorCode>(9999));

    EXPECT_EQ(info.code, ErrorCode::Unknown);
    EXPECT_EQ(sprout::errorCodeName(static_cast<ErrorCode>(9999)), "E_UNKNOWN");
    EXPECT_FALSE(sprout::isKnownErrorCode(static_cast<ErrorCode>(9999)));
    EXPECT_FALSE(sprout::isKnownErrorCode(static_cast<ErrorCode>(-1)));
    EXPECT_FALSE(sprout::errorInfo(static_cast<ErrorCode>(9999)).what.empty());
}

TEST(ErrorCatalogTests, FormatErrorMessage_ContainsWhatWhereHowAndLocation)
{
    const std::string text = sprout::formatErrorMessage(ErrorCode::NotFound, "Assets/player.png");

    EXPECT_NE(text.find("E_NOT_FOUND"), std::string::npos);
    EXPECT_NE(text.find("何が:"), std::string::npos);
    EXPECT_NE(text.find("どこで:"), std::string::npos);
    EXPECT_NE(text.find("どうすれば:"), std::string::npos);
    EXPECT_NE(text.find(std::string{sprout::errorInfo(ErrorCode::NotFound).what}), std::string::npos);
    EXPECT_NE(text.find(std::string{sprout::errorInfo(ErrorCode::NotFound).where}), std::string::npos);
    EXPECT_NE(text.find(std::string{sprout::errorInfo(ErrorCode::NotFound).howToFix}), std::string::npos);
    EXPECT_NE(text.find("Assets/player.png"), std::string::npos);
}

TEST(ErrorCatalogTests, FormatErrorMessage_WithoutLocation_OmitsLocationParentheses)
{
    const std::string text = sprout::formatErrorMessage(ErrorCode::Timeout);

    EXPECT_NE(text.find("E_TIMEOUT"), std::string::npos);
    EXPECT_EQ(text.find("（）"), std::string::npos);
}

TEST(ErrorCatalogTests, FormatErrorMessage_FromInfo_UsesInfoContent)
{
    constexpr ErrorInfo info{ErrorCode::PermissionDenied, "テストの何が", "テストのどこで", "テストのどうすれば"};
    const std::string text = sprout::formatErrorMessage(info, "save.json:12");

    EXPECT_NE(text.find("E_PERMISSION_DENIED"), std::string::npos);
    EXPECT_NE(text.find("テストの何が"), std::string::npos);
    EXPECT_NE(text.find("テストのどこで"), std::string::npos);
    EXPECT_NE(text.find("テストのどうすれば"), std::string::npos);
    EXPECT_NE(text.find("save.json:12"), std::string::npos);
}

TEST(AssertTests, DefaultHandler_IsNull)
{
    sprout::setAssertHandler(nullptr);

    EXPECT_EQ(sprout::assertHandler(), nullptr);
}

TEST(AssertTests, AssertWithTrueCondition_DoesNotCallHandler)
{
    ScopedAssertHandler guard{captureAssert};
    gCaptured = {};
    volatile bool condition = true;

    SPROUT_ASSERT(condition);

    EXPECT_FALSE(gCaptured.called);
}

TEST(AssertTests, AssertWithFalseCondition_CallsHandlerWithContext)
{
    ScopedAssertHandler guard{captureAssert};
    gCaptured = {};
    volatile int value = 1;

    const int expectedLine = __LINE__ + 1;
    SPROUT_ASSERT(value == 2);

    EXPECT_TRUE(gCaptured.called);
    EXPECT_EQ(gCaptured.expression, "value == 2");
    EXPECT_EQ(gCaptured.line, expectedLine);
    EXPECT_NE(gCaptured.file.find("ErrorCatalogTests.cpp"), std::string::npos);
    EXPECT_EQ(gCaptured.message, "条件が満たされませんでした");
    EXPECT_EQ(gCaptured.code, ErrorCode::InvalidState);
    EXPECT_NE(gCaptured.message.find("条件"), std::string::npos);
}

TEST(AssertTests, AssertMsg_PassesCustomJapaneseMessage)
{
    ScopedAssertHandler guard{captureAssert};
    gCaptured = {};
    const std::string message = "体力が 0 以下です";
    volatile bool alive = false;

    SPROUT_ASSERT_MSG(alive, message);

    EXPECT_TRUE(gCaptured.called);
    EXPECT_EQ(gCaptured.message, message);
    EXPECT_EQ(gCaptured.expression, "alive");
    EXPECT_EQ(gCaptured.code, ErrorCode::InvalidState);
}

TEST(AssertTests, FormatAssertFailure_ContainsJapaneseErrorLocationAndCondition)
{
    const sprout::AssertFailure failure{
        "health > 0", "Player.cpp", 42, "体力が 0 以下です", ErrorCode::InvalidState};
    const std::string text = sprout::formatAssertFailure(failure);

    EXPECT_NE(text.find("E_INVALID_STATE"), std::string::npos);
    EXPECT_NE(text.find("何が:"), std::string::npos);
    EXPECT_NE(text.find("どこで:"), std::string::npos);
    EXPECT_NE(text.find("どうすれば:"), std::string::npos);
    EXPECT_NE(text.find("Player.cpp:42"), std::string::npos);
    EXPECT_NE(text.find("条件: health > 0"), std::string::npos);
    EXPECT_NE(text.find("詳細: 体力が 0 以下です"), std::string::npos);
}

TEST(AssertTests, ReportAssertFailure_WithHandler_ReportsFailure)
{
    ScopedAssertHandler guard{captureAssert};
    gCaptured = {};

    sprout::reportAssertFailure({"x", "file.cpp", 7, "メッセージ", ErrorCode::OutOfRange});

    EXPECT_TRUE(gCaptured.called);
    EXPECT_EQ(gCaptured.line, 7);
    EXPECT_EQ(gCaptured.code, ErrorCode::OutOfRange);
}
