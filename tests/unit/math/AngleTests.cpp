#include "sprout/math/Angle.hpp"

#include <gtest/gtest.h>
#include <numbers>
#include <type_traits>

namespace {

using sprout::Angle;
using sprout::Degrees;
using sprout::Radians;

constexpr float kTolerance = 1.0e-5F;

TEST(AngleTests, Degrees_IsStrongType)
{
    static_assert(!std::is_convertible_v<float, Degrees>);
    static_assert(!std::is_convertible_v<Degrees, float>);
    static_assert(std::is_constructible_v<Degrees, float>);
    static_assert(std::is_trivially_copyable_v<Degrees>);
}

TEST(AngleTests, Radians_IsStrongType)
{
    static_assert(!std::is_convertible_v<float, Radians>);
    static_assert(!std::is_convertible_v<Radians, float>);
    static_assert(std::is_constructible_v<Radians, float>);
    static_assert(std::is_trivially_copyable_v<Radians>);
}

TEST(AngleTests, Angle_RequiresExplicitUnit)
{
    static_assert(!std::is_convertible_v<float, Angle>);
    static_assert(!std::is_convertible_v<Degrees, Angle>);
    static_assert(!std::is_convertible_v<Radians, Angle>);
    static_assert(std::is_constructible_v<Angle, Degrees>);
    static_assert(std::is_constructible_v<Angle, Radians>);
}

TEST(AngleTests, DegreesToRadians_KnownAngles_ReturnsExpectedValues)
{
    EXPECT_NEAR(Degrees{0.0F}.toRadians().value, 0.0F, kTolerance);
    EXPECT_NEAR(Degrees{90.0F}.toRadians().value, std::numbers::pi_v<float> * 0.5F, kTolerance);
    EXPECT_NEAR(Degrees{180.0F}.toRadians().value, std::numbers::pi_v<float>, kTolerance);
    EXPECT_NEAR(Degrees{360.0F}.toRadians().value, std::numbers::pi_v<float> * 2.0F, kTolerance);
    EXPECT_NEAR(Degrees{-90.0F}.toRadians().value, std::numbers::pi_v<float> * -0.5F, kTolerance);
}

TEST(AngleTests, RadiansToDegrees_KnownAngles_ReturnsExpectedValues)
{
    EXPECT_NEAR(Radians{0.0F}.toDegrees().value, 0.0F, kTolerance);
    EXPECT_NEAR(Radians{std::numbers::pi_v<float> * 0.5F}.toDegrees().value, 90.0F, kTolerance);
    EXPECT_NEAR(Radians{std::numbers::pi_v<float>}.toDegrees().value, 180.0F, kTolerance);
    EXPECT_NEAR(Radians{std::numbers::pi_v<float> * 2.0F}.toDegrees().value, 360.0F, kTolerance);
    EXPECT_NEAR(Radians{std::numbers::pi_v<float> * -0.5F}.toDegrees().value, -90.0F, kTolerance);
}

TEST(AngleTests, UnitConversion_RoundTrip_RestoresOriginalValue)
{
    EXPECT_NEAR(Radians{1.234F}.toDegrees().toRadians().value, 1.234F, kTolerance);
    EXPECT_NEAR(Degrees{30.0F}.toRadians().toDegrees().value, 30.0F, kTolerance);
}

TEST(AngleTests, Constructor_FromRadians_StoresRadians)
{
    const Angle angle{Radians{std::numbers::pi_v<float> * 0.5F}};

    EXPECT_NEAR(angle.radians(), std::numbers::pi_v<float> * 0.5F, kTolerance);
    EXPECT_NEAR(angle.degrees(), 90.0F, kTolerance);
}

TEST(AngleTests, Constructor_FromDegrees_ConvertsToRadians)
{
    const Angle angle{Degrees{180.0F}};

    EXPECT_NEAR(angle.radians(), std::numbers::pi_v<float>, kTolerance);
    EXPECT_NEAR(angle.degrees(), 180.0F, kTolerance);
}

TEST(AngleTests, Factories_FromValues_CreateEquivalentAngles)
{
    const Angle fromDegrees = Angle::fromDegrees(90.0F);
    const Angle fromRadians = Angle::fromRadians(std::numbers::pi_v<float> * 0.5F);

    EXPECT_NEAR(fromDegrees.radians(), fromRadians.radians(), kTolerance);
    EXPECT_NEAR(fromDegrees.degrees(), 90.0F, kTolerance);
    EXPECT_NEAR(fromRadians.degrees(), 90.0F, kTolerance);
}

TEST(AngleTests, Factories_FromUnitTypes_CreateEquivalentAngles)
{
    const Angle fromDegrees = Angle::fromDegrees(Degrees{90.0F});
    const Angle fromRadians = Angle::fromRadians(Radians{std::numbers::pi_v<float> * 0.5F});

    EXPECT_NEAR(fromDegrees.radians(), fromRadians.radians(), kTolerance);
}

TEST(AngleTests, Accessors_ReturnBothUnits)
{
    const Angle angle = Angle::fromDegrees(45.0F);

    EXPECT_NEAR(angle.asRadians().value, std::numbers::pi_v<float> * 0.25F, kTolerance);
    EXPECT_NEAR(angle.asDegrees().value, 45.0F, kTolerance);
}

TEST(AngleTests, DefaultConstructor_NoArgs_IsZero)
{
    const Angle angle;

    EXPECT_FLOAT_EQ(angle.radians(), 0.0F);
    EXPECT_FLOAT_EQ(angle.degrees(), 0.0F);
}

TEST(AngleTests, Zero_ReturnsZeroAngle)
{
    EXPECT_FLOAT_EQ(Angle::zero().radians(), 0.0F);
}

TEST(AngleTests, AddAndSubtract_TwoAngles_ReturnsExpectedAngle)
{
    EXPECT_NEAR((Angle::fromDegrees(30.0F) + Angle::fromDegrees(60.0F)).degrees(), 90.0F, kTolerance);
    EXPECT_NEAR((Angle::fromDegrees(30.0F) - Angle::fromDegrees(60.0F)).degrees(), -30.0F, kTolerance);
}

TEST(AngleTests, Negate_Angle_FlipsSign)
{
    EXPECT_NEAR((-Angle::fromDegrees(90.0F)).degrees(), -90.0F, kTolerance);
}

TEST(AngleTests, MultiplyByScalar_BothOrders_ScalesAngle)
{
    const Angle left = Angle::fromDegrees(30.0F) * 2.0F;
    const Angle right = 2.0F * Angle::fromDegrees(30.0F);

    EXPECT_NEAR(left.degrees(), 60.0F, kTolerance);
    EXPECT_TRUE(left == right);
}

TEST(AngleTests, DivideByScalar_NonZero_DividesAngle)
{
    EXPECT_NEAR((Angle::fromDegrees(90.0F) / 2.0F).degrees(), 45.0F, kTolerance);
}

TEST(AngleTests, DivideByZero_ReturnsZeroAngle)
{
    EXPECT_FLOAT_EQ((Angle::fromDegrees(90.0F) / 0.0F).radians(), 0.0F);
}

TEST(AngleTests, CompoundAssignment_EachOperator_UpdatesAngle)
{
    Angle angle = Angle::fromDegrees(30.0F);

    angle += Angle::fromDegrees(60.0F);
    EXPECT_NEAR(angle.degrees(), 90.0F, kTolerance);

    angle -= Angle::fromDegrees(30.0F);
    EXPECT_NEAR(angle.degrees(), 60.0F, kTolerance);

    angle *= 2.0F;
    EXPECT_NEAR(angle.degrees(), 120.0F, kTolerance);

    angle /= 2.0F;
    EXPECT_NEAR(angle.degrees(), 60.0F, kTolerance);

    angle /= 0.0F;
    EXPECT_FLOAT_EQ(angle.radians(), 0.0F);
}

TEST(AngleTests, Wrapped_AboveFullTurn_ReturnsEquivalentAngle)
{
    EXPECT_NEAR(Angle::fromDegrees(450.0F).wrapped().degrees(), 90.0F, kTolerance);
}

TEST(AngleTests, Wrapped_NegativeAngle_ReturnsPositiveEquivalent)
{
    EXPECT_NEAR(Angle::fromDegrees(-90.0F).wrapped().degrees(), 270.0F, kTolerance);
}

TEST(AngleTests, Wrapped_FullTurn_ReturnsZero)
{
    const Angle angle = Angle::fromRadians(Angle::kTwoPi).wrapped();

    EXPECT_FLOAT_EQ(angle.radians(), 0.0F);
}

TEST(AngleTests, Wrapped_InRangeAngle_IsUnchanged)
{
    EXPECT_NEAR(Angle::fromDegrees(45.0F).wrapped().degrees(), 45.0F, kTolerance);
}

TEST(AngleTests, Equality_SameAngle_IsTrue)
{
    EXPECT_TRUE(Angle::fromDegrees(90.0F) == Angle::fromDegrees(90.0F));
    EXPECT_TRUE(Degrees{90.0F} == Degrees{90.0F});
    EXPECT_TRUE(Radians{1.5F} == Radians{1.5F});
}

TEST(AngleTests, Equality_DifferentAngle_IsFalse)
{
    EXPECT_FALSE(Angle::fromDegrees(90.0F) == Angle::fromDegrees(91.0F));
    EXPECT_TRUE(Angle::fromDegrees(90.0F) != Angle::fromDegrees(91.0F));
}

}  // namespace
