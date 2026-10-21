#include "sprout/math/Easing.hpp"

#include <gtest/gtest.h>

namespace {

constexpr float kTolerance = 1.0e-5F;

struct EasingCase {
    const char* name;
    float (*function)(float);
};

const EasingCase kAllEasings[] = {
    {"easeInSine", sprout::easeInSine},
    {"easeOutSine", sprout::easeOutSine},
    {"easeInOutSine", sprout::easeInOutSine},
    {"easeInQuad", sprout::easeInQuad},
    {"easeOutQuad", sprout::easeOutQuad},
    {"easeInOutQuad", sprout::easeInOutQuad},
    {"easeInCubic", sprout::easeInCubic},
    {"easeOutCubic", sprout::easeOutCubic},
    {"easeInOutCubic", sprout::easeInOutCubic},
    {"easeInQuart", sprout::easeInQuart},
    {"easeOutQuart", sprout::easeOutQuart},
    {"easeInOutQuart", sprout::easeInOutQuart},
    {"easeInQuint", sprout::easeInQuint},
    {"easeOutQuint", sprout::easeOutQuint},
    {"easeInOutQuint", sprout::easeInOutQuint},
    {"easeInExpo", sprout::easeInExpo},
    {"easeOutExpo", sprout::easeOutExpo},
    {"easeInOutExpo", sprout::easeInOutExpo},
    {"easeInCirc", sprout::easeInCirc},
    {"easeOutCirc", sprout::easeOutCirc},
    {"easeInOutCirc", sprout::easeInOutCirc},
    {"easeInBack", sprout::easeInBack},
    {"easeOutBack", sprout::easeOutBack},
    {"easeInOutBack", sprout::easeInOutBack},
    {"easeInElastic", sprout::easeInElastic},
    {"easeOutElastic", sprout::easeOutElastic},
    {"easeInOutElastic", sprout::easeInOutElastic},
    {"easeInBounce", sprout::easeInBounce},
    {"easeOutBounce", sprout::easeOutBounce},
    {"easeInOutBounce", sprout::easeInOutBounce},
};

struct EasingPair {
    const char* name;
    float (*easeIn)(float);
    float (*easeOut)(float);
};

const EasingPair kEasingPairs[] = {
    {"Sine", sprout::easeInSine, sprout::easeOutSine},
    {"Quad", sprout::easeInQuad, sprout::easeOutQuad},
    {"Cubic", sprout::easeInCubic, sprout::easeOutCubic},
    {"Quart", sprout::easeInQuart, sprout::easeOutQuart},
    {"Quint", sprout::easeInQuint, sprout::easeOutQuint},
    {"Expo", sprout::easeInExpo, sprout::easeOutExpo},
    {"Circ", sprout::easeInCirc, sprout::easeOutCirc},
    {"Back", sprout::easeInBack, sprout::easeOutBack},
    {"Elastic", sprout::easeInElastic, sprout::easeOutElastic},
    {"Bounce", sprout::easeInBounce, sprout::easeOutBounce},
};

struct EasingInOutCase {
    const char* name;
    float (*function)(float);
};

const EasingInOutCase kInOutEasings[] = {
    {"easeInOutSine", sprout::easeInOutSine},
    {"easeInOutQuad", sprout::easeInOutQuad},
    {"easeInOutCubic", sprout::easeInOutCubic},
    {"easeInOutQuart", sprout::easeInOutQuart},
    {"easeInOutQuint", sprout::easeInOutQuint},
    {"easeInOutExpo", sprout::easeInOutExpo},
    {"easeInOutCirc", sprout::easeInOutCirc},
    {"easeInOutBack", sprout::easeInOutBack},
    {"easeInOutElastic", sprout::easeInOutElastic},
    {"easeInOutBounce", sprout::easeInOutBounce},
};

TEST(EasingTests, AllFunctions_AtZero_ReturnZero)
{
    for (const EasingCase& easing : kAllEasings) {
        SCOPED_TRACE(easing.name);
        EXPECT_NEAR(easing.function(0.0F), 0.0F, kTolerance);
    }
}

TEST(EasingTests, AllFunctions_AtOne_ReturnOne)
{
    for (const EasingCase& easing : kAllEasings) {
        SCOPED_TRACE(easing.name);
        EXPECT_NEAR(easing.function(1.0F), 1.0F, kTolerance);
    }
}

TEST(EasingTests, AllFunctions_BelowZero_ClampToStartValue)
{
    for (const EasingCase& easing : kAllEasings) {
        SCOPED_TRACE(easing.name);
        EXPECT_NEAR(easing.function(-0.5F), easing.function(0.0F), kTolerance);
    }
}

TEST(EasingTests, AllFunctions_AboveOne_ClampToEndValue)
{
    for (const EasingCase& easing : kAllEasings) {
        SCOPED_TRACE(easing.name);
        EXPECT_NEAR(easing.function(1.5F), easing.function(1.0F), kTolerance);
    }
}

TEST(EasingTests, InOutFunctions_AtHalf_ReturnHalf)
{
    for (const EasingInOutCase& easing : kInOutEasings) {
        SCOPED_TRACE(easing.name);
        EXPECT_NEAR(easing.function(0.5F), 0.5F, kTolerance);
    }
}

TEST(EasingTests, InAndOutFunctions_AreMirrored)
{
    const float samples[] = {0.1F, 0.25F, 0.37F, 0.6F, 0.9F};
    for (const EasingPair& pair : kEasingPairs) {
        SCOPED_TRACE(pair.name);
        for (const float t : samples) {
            EXPECT_NEAR(pair.easeOut(t), 1.0F - pair.easeIn(1.0F - t), kTolerance);
        }
    }
}

TEST(EasingTests, Sine_AtHalf_MatchesTrigonometricValues)
{
    EXPECT_NEAR(sprout::easeInSine(0.5F), 0.29289322F, kTolerance);
    EXPECT_NEAR(sprout::easeOutSine(0.5F), 0.70710678F, kTolerance);
    EXPECT_NEAR(sprout::easeInOutSine(0.5F), 0.5F, kTolerance);
}

TEST(EasingTests, PolynomialFunctions_AtHalf_ReturnExactValues)
{
    EXPECT_NEAR(sprout::easeInQuad(0.5F), 0.25F, kTolerance);
    EXPECT_NEAR(sprout::easeOutQuad(0.5F), 0.75F, kTolerance);
    EXPECT_NEAR(sprout::easeInCubic(0.5F), 0.125F, kTolerance);
    EXPECT_NEAR(sprout::easeOutCubic(0.5F), 0.875F, kTolerance);
    EXPECT_NEAR(sprout::easeInQuart(0.5F), 0.0625F, kTolerance);
    EXPECT_NEAR(sprout::easeOutQuart(0.5F), 0.9375F, kTolerance);
    EXPECT_NEAR(sprout::easeInQuint(0.5F), 0.03125F, kTolerance);
    EXPECT_NEAR(sprout::easeOutQuint(0.5F), 0.96875F, kTolerance);
}

TEST(EasingTests, Expo_AtHalf_ReturnsExponentialValues)
{
    EXPECT_NEAR(sprout::easeInExpo(0.5F), 0.03125F, kTolerance);
    EXPECT_NEAR(sprout::easeOutExpo(0.5F), 0.96875F, kTolerance);
    EXPECT_NEAR(sprout::easeInOutExpo(0.5F), 0.5F, kTolerance);
}

TEST(EasingTests, Circ_AtHalf_MatchesCircleValues)
{
    EXPECT_NEAR(sprout::easeInCirc(0.5F), 0.1339746F, kTolerance);
    EXPECT_NEAR(sprout::easeOutCirc(0.5F), 0.8660254F, kTolerance);
    EXPECT_NEAR(sprout::easeInOutCirc(0.5F), 0.5F, kTolerance);
}

TEST(EasingTests, Back_OvershootsOutsideUnitRange)
{
    EXPECT_LT(sprout::easeInBack(0.2F), 0.0F);
    EXPECT_GT(sprout::easeOutBack(0.8F), 1.0F);
    EXPECT_NEAR(sprout::easeInBack(0.5F), -0.0876975F, kTolerance);
    EXPECT_NEAR(sprout::easeOutBack(0.5F), 1.0876975F, kTolerance);
}

TEST(EasingTests, Elastic_OvershootsOutsideUnitRange)
{
    EXPECT_LT(sprout::easeInElastic(0.2F), 0.0F);
    EXPECT_GT(sprout::easeOutElastic(0.8F), 1.0F);
    EXPECT_NEAR(sprout::easeInElastic(0.5F), -0.015625F, kTolerance);
    EXPECT_NEAR(sprout::easeOutElastic(0.5F), 1.015625F, kTolerance);
}

TEST(EasingTests, Bounce_IsContinuousAtSegmentBoundaries)
{
    EXPECT_NEAR(sprout::easeOutBounce(1.0F / 2.75F), 1.0F, kTolerance);
    EXPECT_NEAR(sprout::easeOutBounce(2.0F / 2.75F), 1.0F, kTolerance);
    EXPECT_NEAR(sprout::easeOutBounce(2.5F / 2.75F), 1.0F, kTolerance);
}

TEST(EasingTests, Bounce_AtHalf_ReturnsSegmentValues)
{
    EXPECT_NEAR(sprout::easeOutBounce(0.5F), 0.765625F, kTolerance);
    EXPECT_NEAR(sprout::easeInBounce(0.5F), 0.234375F, kTolerance);
    EXPECT_NEAR(sprout::easeInOutBounce(0.5F), 0.5F, kTolerance);
}

}  // namespace
