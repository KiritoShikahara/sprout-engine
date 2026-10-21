#include "sprout/math/MathUtil.hpp"

#include <gtest/gtest.h>
#include <limits>

namespace {

constexpr float kTolerance = 1.0e-5F;

TEST(MathUtilTests, Lerp_StartEndAndMidpoint_ReturnsInterpolatedValues)
{
    EXPECT_FLOAT_EQ(sprout::lerp(10.0F, 20.0F, 0.0F), 10.0F);
    EXPECT_FLOAT_EQ(sprout::lerp(10.0F, 20.0F, 1.0F), 20.0F);
    EXPECT_FLOAT_EQ(sprout::lerp(10.0F, 20.0F, 0.5F), 15.0F);
}

TEST(MathUtilTests, Lerp_OutsideUnitInterval_Extrapolates)
{
    EXPECT_FLOAT_EQ(sprout::lerp(10.0F, 20.0F, 2.0F), 30.0F);
    EXPECT_FLOAT_EQ(sprout::lerp(10.0F, 20.0F, -1.0F), 0.0F);
}

TEST(MathUtilTests, InverseLerp_KnownValues_ReturnsRatio)
{
    EXPECT_FLOAT_EQ(sprout::inverseLerp(0.0F, 100.0F, 25.0F), 0.25F);
    EXPECT_FLOAT_EQ(sprout::inverseLerp(0.0F, 100.0F, 0.0F), 0.0F);
    EXPECT_FLOAT_EQ(sprout::inverseLerp(0.0F, 100.0F, 100.0F), 1.0F);
}

TEST(MathUtilTests, InverseLerp_OutsideRange_Extrapolates)
{
    EXPECT_FLOAT_EQ(sprout::inverseLerp(0.0F, 100.0F, 150.0F), 1.5F);
    EXPECT_FLOAT_EQ(sprout::inverseLerp(0.0F, 100.0F, -50.0F), -0.5F);
}

TEST(MathUtilTests, InverseLerp_EqualEndpoints_ReturnsZero)
{
    const float result = sprout::inverseLerp(5.0F, 5.0F, 5.0F);

    EXPECT_FLOAT_EQ(result, 0.0F);
    EXPECT_FALSE(std::isnan(result));
}

TEST(MathUtilTests, Clamp_BelowMinimum_ReturnsMinimum)
{
    EXPECT_FLOAT_EQ(sprout::clamp(-3.0F, 0.0F, 10.0F), 0.0F);
}

TEST(MathUtilTests, Clamp_WithinRange_ReturnsValue)
{
    EXPECT_FLOAT_EQ(sprout::clamp(4.0F, 0.0F, 10.0F), 4.0F);
}

TEST(MathUtilTests, Clamp_AboveMaximum_ReturnsMaximum)
{
    EXPECT_FLOAT_EQ(sprout::clamp(12.0F, 0.0F, 10.0F), 10.0F);
}

TEST(MathUtilTests, Clamp_OnBoundaries_ReturnsBoundary)
{
    EXPECT_FLOAT_EQ(sprout::clamp(0.0F, 0.0F, 10.0F), 0.0F);
    EXPECT_FLOAT_EQ(sprout::clamp(10.0F, 0.0F, 10.0F), 10.0F);
}

TEST(MathUtilTests, ApproxEqual_WithinDefaultTolerance_IsTrue)
{
    EXPECT_TRUE(sprout::approxEqual(1.0F, 1.0F + 5.0e-6F));
    EXPECT_TRUE(sprout::approxEqual(-2.0F, -2.0F - 5.0e-6F));
}

TEST(MathUtilTests, ApproxEqual_OutsideDefaultTolerance_IsFalse)
{
    EXPECT_FALSE(sprout::approxEqual(1.0F, 1.001F));
}

TEST(MathUtilTests, ApproxEqual_CustomTolerance_UsesGivenValue)
{
    EXPECT_TRUE(sprout::approxEqual(1.0F, 1.5F, 0.5F));
    EXPECT_FALSE(sprout::approxEqual(1.0F, 1.5F, 0.25F));
}

TEST(MathUtilTests, MoveTowards_WithinStep_ReturnsTarget)
{
    EXPECT_FLOAT_EQ(sprout::moveTowards(1.0F, 2.0F, 2.0F), 2.0F);
    EXPECT_FLOAT_EQ(sprout::moveTowards(1.0F, 2.0F, 1.0F), 2.0F);
}

TEST(MathUtilTests, MoveTowards_ExceedsStep_MovesInDirection)
{
    EXPECT_FLOAT_EQ(sprout::moveTowards(0.0F, 10.0F, 3.0F), 3.0F);
    EXPECT_FLOAT_EQ(sprout::moveTowards(10.0F, 0.0F, 3.0F), 7.0F);
}

TEST(MathUtilTests, MoveTowards_ZeroStep_ReturnsCurrent)
{
    EXPECT_FLOAT_EQ(sprout::moveTowards(4.0F, 10.0F, 0.0F), 4.0F);
    EXPECT_FLOAT_EQ(sprout::moveTowards(4.0F, 10.0F, -1.0F), 4.0F);
}

TEST(MathUtilTests, SmoothDamp_ZeroSmoothTime_SnapsToTarget)
{
    float velocity = 5.0F;

    const float result = sprout::smoothDamp(0.0F, 10.0F, velocity, 0.0F, 1.0F / 60.0F);

    EXPECT_FLOAT_EQ(result, 10.0F);
    EXPECT_FLOAT_EQ(velocity, 0.0F);
}

TEST(MathUtilTests, SmoothDamp_ZeroDeltaTime_KeepsCurrent)
{
    float velocity = 2.0F;

    const float result = sprout::smoothDamp(3.0F, 10.0F, velocity, 0.3F, 0.0F);

    EXPECT_FLOAT_EQ(result, 3.0F);
    EXPECT_FLOAT_EQ(velocity, 2.0F);
}

TEST(MathUtilTests, SmoothDamp_ApproachesTargetWithoutOvershoot)
{
    constexpr float deltaTime = 1.0F / 60.0F;
    constexpr float target = 10.0F;

    float velocity = 0.0F;
    float current = 0.0F;
    for (int step = 0; step < 600; ++step) {
        const float previous = current;
        current = sprout::smoothDamp(current, target, velocity, 0.3F, deltaTime);
        EXPECT_GE(current, previous);
        EXPECT_LE(current, target + kTolerance);
    }

    EXPECT_NEAR(current, target, 1.0e-3F);
    EXPECT_NEAR(velocity, 0.0F, 1.0e-3F);
}

TEST(MathUtilTests, SmoothDamp_WithMaxSpeed_StillApproachesTarget)
{
    constexpr float deltaTime = 1.0F / 60.0F;
    constexpr float target = 100.0F;

    float velocity = 0.0F;
    const float first = sprout::smoothDamp(0.0F, target, velocity, 0.5F, 20.0F, deltaTime);

    EXPECT_GT(first, 0.0F);
    EXPECT_LT(first, target);

    float current = first;
    for (int step = 0; step < 600; ++step) {
        current = sprout::smoothDamp(current, target, velocity, 0.5F, 20.0F, deltaTime);
    }

    EXPECT_NEAR(current, target, 1.0e-3F);
}

TEST(MathUtilTests, SmoothDamp_AlreadyAtTargetAndStopped_DoesNotMove)
{
    float velocity = 0.0F;

    const float result = sprout::smoothDamp(10.0F, 10.0F, velocity, 0.3F, 1.0F / 60.0F);

    EXPECT_FLOAT_EQ(result, 10.0F);
    EXPECT_FLOAT_EQ(velocity, 0.0F);
}

}  // namespace
