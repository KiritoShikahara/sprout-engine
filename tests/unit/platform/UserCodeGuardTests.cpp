#include "platform/UserCodeGuard.hpp"

#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

namespace {

using sprout::detail::runUserCode;
using sprout::detail::UserCodeFault;
using sprout::Error;
using sprout::ErrorCode;

TEST(UserCodeGuardTests, RunUserCode_Success_ReturnsOk)
{
    int value = 0;
    const auto result = runUserCode([&value] { value = 42; });

    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(value, 42);
}

TEST(UserCodeGuardTests, RunUserCode_StdException_ReturnsErrorWithMessage)
{
    const auto result = runUserCode([] {
        throw std::runtime_error("intentional error");
    });

    EXPECT_FALSE(result.isOk());
    EXPECT_EQ(result.error().code(), ErrorCode::Unknown);
    EXPECT_NE(result.error().message().find("intentional error"), std::string::npos);
    EXPECT_NE(result.error().message().find("User code fault"), std::string::npos);
}

TEST(UserCodeGuardTests, RunUserCode_StdLogicError_ReturnsErrorWithMessage)
{
    const auto result = runUserCode([] {
        throw std::logic_error("logic failed");
    });

    EXPECT_FALSE(result.isOk());
    EXPECT_NE(result.error().message().find("logic failed"), std::string::npos);
}

TEST(UserCodeGuardTests, RunUserCode_UnknownException_ReturnsError)
{
    const auto result = runUserCode([] {
        throw 42;  // std::exception 派生でない
    });

    EXPECT_FALSE(result.isOk());
    EXPECT_NE(result.error().message().find("Unknown exception"), std::string::npos);
}

TEST(UserCodeGuardTests, RunUserCode_NestedCall_CatchesException)
{
    auto inner = [] { throw std::invalid_argument("nested"); };
    auto outer = [&inner] { inner(); };

    const auto result = runUserCode(outer);

    EXPECT_FALSE(result.isOk());
    EXPECT_NE(result.error().message().find("nested"), std::string::npos);
}

TEST(UserCodeGuardTests, RunUserCode_LambdaWithCapture_Works)
{
    int captured = 10;
    const auto result = runUserCode([captured]() mutable {
        captured += 5;
    });

    EXPECT_TRUE(result.isOk());
}

TEST(UserCodeGuardTests, SEHExceptionDescription_KnownCodes_HaveDescriptions)
{
    using sprout::detail::sehExceptionDescription;

#if defined(_WIN32)
    EXPECT_NE(sehExceptionDescription(EXCEPTION_ACCESS_VIOLATION).find("Access violation"), std::string::npos);
    EXPECT_NE(sehExceptionDescription(EXCEPTION_STACK_OVERFLOW).find("Stack overflow"), std::string::npos);
    EXPECT_NE(sehExceptionDescription(EXCEPTION_INT_DIVIDE_BY_ZERO).find("Integer divide by zero"), std::string::npos);
    EXPECT_NE(sehExceptionDescription(EXCEPTION_ILLEGAL_INSTRUCTION).find("Illegal instruction"), std::string::npos);
#else
    // 非 Windows では全コードが "SEH not supported" を返す
    EXPECT_EQ(sehExceptionDescription(1), "SEH not supported on this platform");
    EXPECT_EQ(sehExceptionDescription(0xC0000005), "SEH not supported on this platform");
#endif
}

TEST(UserCodeGuardTests, SEHExceptionDescription_UnknownCode_ReturnsHexFormat)
{
    using sprout::detail::sehExceptionDescription;

#if defined(_WIN32)
    const std::string desc = sehExceptionDescription(0xC0000099);
    EXPECT_NE(desc.find("0xC0000099"), std::string::npos);
    EXPECT_NE(desc.find("Unknown SEH exception"), std::string::npos);
#else
    const std::string desc = sehExceptionDescription(0xC0000099);
    EXPECT_EQ(desc, "SEH not supported on this platform");
#endif
}

#if defined(_WIN32)
// Windows SEH テストは実行時のクラッシュを避けるため、ここではスキップする。
// 実際の SEH 例外（アクセス違反等）を発生させるとプロセスが不安定になるため。
// 統合テスト（UserCodeFaultTests.cpp）で実際の障害注入を行う。
TEST(UserCodeGuardTests, DISABLED_RunUserCode_AccessViolation_ReturnsError)
{
    // 意図的にアクセス違反を起こすコードはテストランナーをクラッシュさせる可能性があるため無効化。
    // 代わりに統合テストで別プロセスとして検証する。
    GTEST_SKIP() << "SEH exception tests require separate process";
}
#endif

}  // namespace