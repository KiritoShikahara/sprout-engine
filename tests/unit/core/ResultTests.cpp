#include "sprout/core/Result.hpp"

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <utility>

namespace {

TEST(ErrorTests, Constructor_CodeAndMessage_StoresBoth)
{
    const sprout::Error error{sprout::ErrorCode::NotFound, "Assets/player.png が見つかりません"};

    EXPECT_EQ(error.code(), sprout::ErrorCode::NotFound);
    EXPECT_EQ(error.message(), "Assets/player.png が見つかりません");
}

TEST(ErrorTests, Constructor_CodeOnly_LeavesMessageEmpty)
{
    const sprout::Error error{sprout::ErrorCode::Io};

    EXPECT_EQ(error.code(), sprout::ErrorCode::Io);
    EXPECT_TRUE(error.message().empty());
}

TEST(ErrorTests, DefaultConstructor_IsUnknown)
{
    const sprout::Error error;

    EXPECT_EQ(error.code(), sprout::ErrorCode::Unknown);
    EXPECT_TRUE(error.message().empty());
}

TEST(ResultTests, Constructor_Value_IsOkAndReturnsValue)
{
    const sprout::Result<int> result{42};

    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(result.value(), 42);
}

TEST(ResultTests, Constructor_Error_IsNotOkAndReturnsError)
{
    const sprout::Result<int> result{sprout::Error{sprout::ErrorCode::Parse, "数値ではありません"}};

    EXPECT_FALSE(result.isOk());
    EXPECT_EQ(result.error().code(), sprout::ErrorCode::Parse);
    EXPECT_EQ(result.error().message(), "数値ではありません");
}

TEST(ResultTests, Value_StringType_RoundTrips)
{
    const sprout::Result<std::string> result{std::string{"Sprout"}};

    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(result.value(), "Sprout");
}

TEST(ResultTests, Value_NonConstAccess_AllowsModification)
{
    sprout::Result<int> result{1};

    result.value() = 7;

    EXPECT_EQ(result.value(), 7);
}

TEST(ResultTests, Value_MoveOnlyType_TransfersOwnership)
{
    sprout::Result<std::unique_ptr<int>> result{std::make_unique<int>(7)};

    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(*result.value(), 7);
    std::unique_ptr<int> moved = std::move(result).value();
    EXPECT_EQ(*moved, 7);
}

TEST(ResultTests, Copy_ValueResult_CopiesValue)
{
    const sprout::Result<int> original{11};

    const sprout::Result<int> copy = original;

    EXPECT_TRUE(copy.isOk());
    EXPECT_EQ(copy.value(), 11);
}

TEST(ResultVoidTests, DefaultConstructor_IsOk)
{
    const sprout::Result<void> result;

    EXPECT_TRUE(result.isOk());
}

TEST(ResultVoidTests, Constructor_Error_IsNotOkAndReturnsError)
{
    const sprout::Result<void> result{sprout::Error{sprout::ErrorCode::PermissionDenied, "書き込めません"}};

    EXPECT_FALSE(result.isOk());
    EXPECT_EQ(result.error().code(), sprout::ErrorCode::PermissionDenied);
    EXPECT_EQ(result.error().message(), "書き込めません");
}

}  // namespace
