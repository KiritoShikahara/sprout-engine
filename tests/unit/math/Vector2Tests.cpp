#include "sprout/math/Vector2.hpp"

#include <cmath>
#include <gtest/gtest.h>
#include <numbers>

namespace {

using sprout::Vector2;
using sprout::Vector2Int;

constexpr float kTolerance = 1.0e-5F;

TEST(Vector2Tests, DefaultConstructor_NoArgs_IsZero)
{
    const Vector2 value;

    EXPECT_FLOAT_EQ(value.x, 0.0F);
    EXPECT_FLOAT_EQ(value.y, 0.0F);
}

TEST(Vector2Tests, Constructor_Components_StoresValues)
{
    const Vector2 value{2.5F, -4.0F};

    EXPECT_FLOAT_EQ(value.x, 2.5F);
    EXPECT_FLOAT_EQ(value.y, -4.0F);
}

TEST(Vector2Tests, Constants_PredefinedVectors_ReturnExpectedValues)
{
    EXPECT_TRUE((Vector2::zero() == Vector2{0.0F, 0.0F}));
    EXPECT_TRUE((Vector2::one() == Vector2{1.0F, 1.0F}));
    EXPECT_TRUE((Vector2::right() == Vector2{1.0F, 0.0F}));
    EXPECT_TRUE((Vector2::up() == Vector2{0.0F, 1.0F}));
}

TEST(Vector2Tests, Add_TwoVectors_AddsComponents)
{
    const Vector2 result = Vector2{1.0F, 2.0F} + Vector2{3.0F, 5.0F};

    EXPECT_FLOAT_EQ(result.x, 4.0F);
    EXPECT_FLOAT_EQ(result.y, 7.0F);
}

TEST(Vector2Tests, Subtract_TwoVectors_SubtractsComponents)
{
    const Vector2 result = Vector2{1.0F, 2.0F} - Vector2{3.0F, 5.0F};

    EXPECT_FLOAT_EQ(result.x, -2.0F);
    EXPECT_FLOAT_EQ(result.y, -3.0F);
}

TEST(Vector2Tests, Negate_Vector_FlipsComponents)
{
    const Vector2 result = -Vector2{1.5F, -2.5F};

    EXPECT_FLOAT_EQ(result.x, -1.5F);
    EXPECT_FLOAT_EQ(result.y, 2.5F);
}

TEST(Vector2Tests, MultiplyByScalar_BothOrders_ScalesComponents)
{
    const Vector2 left = Vector2{1.0F, -2.0F} * 3.0F;
    const Vector2 right = 3.0F * Vector2{1.0F, -2.0F};

    EXPECT_FLOAT_EQ(left.x, 3.0F);
    EXPECT_FLOAT_EQ(left.y, -6.0F);
    EXPECT_TRUE(left == right);
}

TEST(Vector2Tests, DivideByScalar_NonZero_DividesComponents)
{
    const Vector2 result = Vector2{4.0F, -6.0F} / 2.0F;

    EXPECT_FLOAT_EQ(result.x, 2.0F);
    EXPECT_FLOAT_EQ(result.y, -3.0F);
}

TEST(Vector2Tests, DivideByZero_ReturnsZeroVector)
{
    const Vector2 result = Vector2{4.0F, -6.0F} / 0.0F;

    EXPECT_TRUE(result == Vector2::zero());
}

TEST(Vector2Tests, CompoundAssignment_EachOperator_UpdatesComponents)
{
    Vector2 value{1.0F, 2.0F};

    value += Vector2{3.0F, 4.0F};
    EXPECT_FLOAT_EQ(value.x, 4.0F);
    EXPECT_FLOAT_EQ(value.y, 6.0F);

    value -= Vector2{1.0F, 1.0F};
    EXPECT_FLOAT_EQ(value.x, 3.0F);
    EXPECT_FLOAT_EQ(value.y, 5.0F);

    value *= 2.0F;
    EXPECT_FLOAT_EQ(value.x, 6.0F);
    EXPECT_FLOAT_EQ(value.y, 10.0F);

    value /= 2.0F;
    EXPECT_FLOAT_EQ(value.x, 3.0F);
    EXPECT_FLOAT_EQ(value.y, 5.0F);

    value /= 0.0F;
    EXPECT_TRUE(value == Vector2::zero());
}

TEST(Vector2Tests, Equality_SameComponents_IsTrue)
{
    EXPECT_TRUE((Vector2{1.0F, 2.0F} == Vector2{1.0F, 2.0F}));
}

TEST(Vector2Tests, Equality_DifferentComponents_IsFalse)
{
    EXPECT_FALSE((Vector2{1.0F, 2.0F} == Vector2{1.0F, 2.5F}));
    EXPECT_TRUE((Vector2{1.0F, 2.0F} != Vector2{1.0F, 2.5F}));
}

TEST(Vector2Tests, LengthSquared_ThreeFour_ReturnsTwentyFive)
{
    EXPECT_FLOAT_EQ((Vector2{3.0F, 4.0F}.lengthSquared()), 25.0F);
}

TEST(Vector2Tests, Length_ThreeFour_ReturnsFive)
{
    EXPECT_FLOAT_EQ((Vector2{3.0F, 4.0F}.length()), 5.0F);
}

TEST(Vector2Tests, Normalized_ThreeFour_ReturnsUnitVector)
{
    const Vector2 result = Vector2{3.0F, 4.0F}.normalized();

    EXPECT_NEAR(result.x, 0.6F, kTolerance);
    EXPECT_NEAR(result.y, 0.8F, kTolerance);
    EXPECT_NEAR(result.length(), 1.0F, kTolerance);
}

TEST(Vector2Tests, Normalized_ZeroVector_ReturnsZeroVector)
{
    const Vector2 result = Vector2::zero().normalized();

    EXPECT_TRUE(result == Vector2::zero());
    EXPECT_FALSE(std::isnan(result.x));
    EXPECT_FALSE(std::isnan(result.y));
}

TEST(Vector2Tests, Dot_PerpendicularVectors_ReturnsZero)
{
    EXPECT_FLOAT_EQ((Vector2{1.0F, 0.0F}.dot(Vector2{0.0F, 1.0F})), 0.0F);
}

TEST(Vector2Tests, Dot_KnownVectors_ReturnsSumOfProducts)
{
    EXPECT_FLOAT_EQ((Vector2{1.0F, 2.0F}.dot(Vector2{3.0F, 4.0F})), 11.0F);
}

TEST(Vector2Tests, Cross_ParallelVectors_ReturnsZero)
{
    EXPECT_FLOAT_EQ((Vector2{2.0F, 4.0F}.cross(Vector2{1.0F, 2.0F})), 0.0F);
}

TEST(Vector2Tests, Cross_BasisVectors_ReturnsOne)
{
    EXPECT_FLOAT_EQ((Vector2::right().cross(Vector2::up())), 1.0F);
    EXPECT_FLOAT_EQ((Vector2::up().cross(Vector2::right())), -1.0F);
}

TEST(Vector2Tests, Rotated_QuarterTurn_ReturnsUnitY)
{
    const Vector2 result = Vector2::right().rotated(std::numbers::pi_v<float> * 0.5F);

    EXPECT_NEAR(result.x, 0.0F, kTolerance);
    EXPECT_NEAR(result.y, 1.0F, kTolerance);
}

TEST(Vector2Tests, Rotated_FullTurn_ReturnsOriginal)
{
    const Vector2 result = Vector2{2.0F, -3.0F}.rotated(std::numbers::pi_v<float> * 2.0F);

    EXPECT_NEAR(result.x, 2.0F, kTolerance);
    EXPECT_NEAR(result.y, -3.0F, kTolerance);
}

TEST(Vector2Tests, Angle_UnitY_ReturnsHalfPi)
{
    EXPECT_NEAR(Vector2::up().angle(), std::numbers::pi_v<float> * 0.5F, kTolerance);
}

TEST(Vector2Tests, IsApproxEqual_WithinTolerance_IsTrue)
{
    EXPECT_TRUE((Vector2{1.0F, 1.0F}.isApproxEqual(Vector2{1.0F + 5.0e-6F, 1.0F})));
    EXPECT_TRUE((Vector2{1.0F, 1.0F}.isApproxEqual(Vector2{1.5F, 1.0F}, 0.5F)));
}

TEST(Vector2Tests, IsApproxEqual_OutsideTolerance_IsFalse)
{
    EXPECT_FALSE((Vector2{1.0F, 1.0F}.isApproxEqual(Vector2{1.001F, 1.0F})));
}

TEST(Vector2IntTests, Constructor_Components_StoresValues)
{
    const Vector2Int value{3, -4};

    EXPECT_EQ(value.x, 3);
    EXPECT_EQ(value.y, -4);
}

TEST(Vector2IntTests, DefaultConstructor_NoArgs_IsZero)
{
    const Vector2Int value;

    EXPECT_TRUE(value.isZero());
}

TEST(Vector2IntTests, AddAndSubtract_TwoVectors_ReturnsComponentWiseResult)
{
    const Vector2Int sum = Vector2Int{1, 2} + Vector2Int{3, 5};
    const Vector2Int difference = Vector2Int{1, 2} - Vector2Int{3, 5};

    EXPECT_TRUE((sum == Vector2Int{4, 7}));
    EXPECT_TRUE((difference == Vector2Int{-2, -3}));
}

TEST(Vector2IntTests, Negate_Vector_FlipsComponents)
{
    EXPECT_TRUE((-Vector2Int{2, -3} == Vector2Int{-2, 3}));
}

TEST(Vector2IntTests, MultiplyByScalar_BothOrders_ScalesComponents)
{
    const Vector2Int left = Vector2Int{2, -3} * 4;
    const Vector2Int right = 4 * Vector2Int{2, -3};

    EXPECT_TRUE((left == Vector2Int{8, -12}));
    EXPECT_TRUE(left == right);
}

TEST(Vector2IntTests, DivideByScalar_NonZero_DividesComponents)
{
    EXPECT_TRUE((Vector2Int{7, -8} / 2 == Vector2Int{3, -4}));
}

TEST(Vector2IntTests, DivideByZero_ReturnsZeroVector)
{
    EXPECT_TRUE((Vector2Int{7, -8} / 0 == Vector2Int::zero()));
}

TEST(Vector2IntTests, CompoundAssignment_EachOperator_UpdatesComponents)
{
    Vector2Int value{1, 2};

    value += Vector2Int{3, 4};
    EXPECT_TRUE((value == Vector2Int{4, 6}));

    value -= Vector2Int{1, 1};
    EXPECT_TRUE((value == Vector2Int{3, 5}));

    value *= 2;
    EXPECT_TRUE((value == Vector2Int{6, 10}));

    value /= 2;
    EXPECT_TRUE((value == Vector2Int{3, 5}));

    value /= 0;
    EXPECT_TRUE(value.isZero());
}

TEST(Vector2IntTests, Dot_KnownVectors_ReturnsSumOfProducts)
{
    EXPECT_EQ((Vector2Int{1, 2}.dot(Vector2Int{3, 4})), 11);
}

TEST(Vector2IntTests, Cross_BasisVectors_ReturnsOne)
{
    EXPECT_EQ((Vector2Int{1, 0}.cross(Vector2Int{0, 1})), 1);
    EXPECT_EQ((Vector2Int{0, 1}.cross(Vector2Int{1, 0})), -1);
}

TEST(Vector2IntTests, LengthSquared_ThreeFour_ReturnsTwentyFive)
{
    EXPECT_EQ((Vector2Int{3, 4}.lengthSquared()), 25);
}

TEST(Vector2IntTests, Length_ThreeFour_ReturnsFive)
{
    EXPECT_FLOAT_EQ((Vector2Int{3, 4}.length()), 5.0F);
}

TEST(Vector2IntTests, Equality_SameComponents_IsTrue)
{
    EXPECT_TRUE((Vector2Int{1, 2} == Vector2Int::one() + Vector2Int{0, 1}));
    EXPECT_TRUE((Vector2Int::one() == Vector2Int{1, 1}));
}

TEST(Vector2IntTests, Equality_DifferentComponents_IsFalse)
{
    EXPECT_FALSE((Vector2Int{1, 2} == Vector2Int{1, 3}));
    EXPECT_TRUE((Vector2Int{1, 2} != Vector2Int{1, 3}));
}

TEST(Vector2IntTests, IsZero_NonZeroVector_IsFalse)
{
    EXPECT_FALSE((Vector2Int{0, 1}.isZero()));
}

}  // namespace
