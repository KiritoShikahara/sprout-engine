#include "sprout/math/Circle.hpp"
#include "sprout/math/Rect.hpp"
#include "sprout/math/Vector2.hpp"

#include <gtest/gtest.h>

namespace {

using sprout::Circle;
using sprout::Rect;
using sprout::Vector2;

constexpr float kTolerance = 1.0e-5F;

TEST(RectTests, DefaultConstructor_NoArgs_IsZero)
{
    const Rect value;

    EXPECT_TRUE(value.position == (Vector2{0.0F, 0.0F}));
    EXPECT_TRUE(value.size == (Vector2{0.0F, 0.0F}));
}

TEST(RectTests, Constructor_PositionAndSize_StoresValues)
{
    const Rect value{Vector2{2.0F, 3.0F}, Vector2{8.0F, 6.0F}};

    EXPECT_TRUE(value.position == (Vector2{2.0F, 3.0F}));
    EXPECT_TRUE(value.size == (Vector2{8.0F, 6.0F}));
}

TEST(RectTests, Constructor_Components_StoresValues)
{
    const Rect value{2.0F, 3.0F, 8.0F, 6.0F};

    EXPECT_TRUE(value.position == (Vector2{2.0F, 3.0F}));
    EXPECT_TRUE(value.size == (Vector2{8.0F, 6.0F}));
}

TEST(RectTests, MinimumAndMaximum_PositionAndSize_ReturnCorners)
{
    const Rect value{Vector2{2.0F, 3.0F}, Vector2{8.0F, 6.0F}};

    EXPECT_TRUE(value.minimum() == (Vector2{2.0F, 3.0F}));
    EXPECT_TRUE(value.maximum() == (Vector2{10.0F, 9.0F}));
}

TEST(RectTests, Center_PositionAndSize_ReturnsMidpoint)
{
    const Rect value{Vector2{2.0F, 3.0F}, Vector2{8.0F, 6.0F}};

    EXPECT_TRUE(value.center() == (Vector2{6.0F, 6.0F}));
}

TEST(RectTests, Contains_PointInside_ReturnsTrue)
{
    const Rect value{Vector2{0.0F, 0.0F}, Vector2{10.0F, 20.0F}};

    EXPECT_TRUE(value.contains(Vector2{5.0F, 10.0F}));
}

TEST(RectTests, Contains_PointOnBoundary_ReturnsTrue)
{
    const Rect value{Vector2{0.0F, 0.0F}, Vector2{10.0F, 20.0F}};

    EXPECT_TRUE(value.contains(Vector2{0.0F, 20.0F}));
    EXPECT_TRUE(value.contains(Vector2{10.0F, 0.0F}));
}

TEST(RectTests, Contains_PointOutside_ReturnsFalse)
{
    const Rect value{Vector2{0.0F, 0.0F}, Vector2{10.0F, 20.0F}};

    EXPECT_FALSE(value.contains(Vector2{-0.1F, 10.0F}));
    EXPECT_FALSE(value.contains(Vector2{10.1F, 10.0F}));
}

TEST(RectTests, Contains_RectFullyInside_ReturnsTrue)
{
    const Rect outer{Vector2{0.0F, 0.0F}, Vector2{10.0F, 10.0F}};
    const Rect inner{Vector2{2.0F, 2.0F}, Vector2{3.0F, 3.0F}};

    EXPECT_TRUE(outer.contains(inner));
}

TEST(RectTests, Contains_SameRect_ReturnsTrue)
{
    const Rect value{Vector2{1.0F, 2.0F}, Vector2{3.0F, 4.0F}};

    EXPECT_TRUE(value.contains(value));
}

TEST(RectTests, Contains_RectPartiallyOutside_ReturnsFalse)
{
    const Rect outer{Vector2{0.0F, 0.0F}, Vector2{10.0F, 10.0F}};
    const Rect other{Vector2{8.0F, 8.0F}, Vector2{4.0F, 4.0F}};

    EXPECT_FALSE(outer.contains(other));
}

TEST(RectTests, Intersects_OverlappingRects_ReturnsTrue)
{
    const Rect first{Vector2{0.0F, 0.0F}, Vector2{10.0F, 10.0F}};
    const Rect second{Vector2{5.0F, 5.0F}, Vector2{10.0F, 10.0F}};

    EXPECT_TRUE(first.intersects(second));
}

TEST(RectTests, Intersects_TouchingEdges_ReturnsTrue)
{
    const Rect first{Vector2{0.0F, 0.0F}, Vector2{10.0F, 10.0F}};
    const Rect second{Vector2{10.0F, 0.0F}, Vector2{5.0F, 10.0F}};

    EXPECT_TRUE(first.intersects(second));
}

TEST(RectTests, Intersects_SeparatedRects_ReturnsFalse)
{
    const Rect first{Vector2{0.0F, 0.0F}, Vector2{10.0F, 10.0F}};
    const Rect second{Vector2{10.1F, 0.0F}, Vector2{5.0F, 10.0F}};

    EXPECT_FALSE(first.intersects(second));
}

TEST(RectTests, Intersects_RectInsideOther_ReturnsTrue)
{
    const Rect outer{Vector2{0.0F, 0.0F}, Vector2{10.0F, 10.0F}};
    const Rect inner{Vector2{2.0F, 2.0F}, Vector2{3.0F, 3.0F}};

    EXPECT_TRUE(outer.intersects(inner));
    EXPECT_TRUE(inner.intersects(outer));
}

TEST(RectTests, United_OverlappingRects_ReturnsBoundingRect)
{
    const Rect first{Vector2{0.0F, 0.0F}, Vector2{10.0F, 10.0F}};
    const Rect second{Vector2{5.0F, 5.0F}, Vector2{10.0F, 10.0F}};

    const Rect result = first.united(second);

    EXPECT_TRUE(result.position == (Vector2{0.0F, 0.0F}));
    EXPECT_TRUE(result.size == (Vector2{15.0F, 15.0F}));
}

TEST(RectTests, United_SeparatedRects_ReturnsBoundingRect)
{
    const Rect first{Vector2{2.0F, 3.0F}, Vector2{4.0F, 5.0F}};
    const Rect second{Vector2{10.0F, 1.0F}, Vector2{2.0F, 2.0F}};

    const Rect result = first.united(second);

    EXPECT_TRUE(result.position == (Vector2{2.0F, 1.0F}));
    EXPECT_TRUE(result.size == (Vector2{10.0F, 7.0F}));
}

TEST(RectTests, Expanded_PositiveAmount_GrowsAllSides)
{
    const Rect value{Vector2{0.0F, 0.0F}, Vector2{10.0F, 20.0F}};

    const Rect result = value.expanded(2.0F);

    EXPECT_TRUE(result.position == (Vector2{-2.0F, -2.0F}));
    EXPECT_TRUE(result.size == (Vector2{14.0F, 24.0F}));
}

TEST(RectTests, Expanded_NegativeAmount_ShrinksAroundCenter)
{
    const Rect value{Vector2{0.0F, 0.0F}, Vector2{10.0F, 20.0F}};

    const Rect result = value.expanded(-2.0F);

    EXPECT_TRUE(result.position == (Vector2{2.0F, 2.0F}));
    EXPECT_TRUE(result.size == (Vector2{6.0F, 16.0F}));
}

TEST(RectTests, Expanded_ShrinkBeyondSize_ClampsToZero)
{
    const Rect value{Vector2{0.0F, 0.0F}, Vector2{4.0F, 8.0F}};

    const Rect result = value.expanded(-10.0F);

    EXPECT_TRUE(result.center() == (Vector2{2.0F, 4.0F}));
    EXPECT_TRUE(result.size == (Vector2{0.0F, 0.0F}));
}

TEST(RectTests, Equality_SameValues_ReturnsTrue)
{
    const Rect first{Vector2{1.0F, 2.0F}, Vector2{3.0F, 4.0F}};
    const Rect second{Vector2{1.0F, 2.0F}, Vector2{3.0F, 4.0F}};

    EXPECT_TRUE(first == second);
}

TEST(CircleTests, DefaultConstructor_NoArgs_IsZero)
{
    const Circle value;

    EXPECT_TRUE(value.center == (Vector2{0.0F, 0.0F}));
    EXPECT_FLOAT_EQ(value.radius, 0.0F);
}

TEST(CircleTests, Constructor_CenterAndRadius_StoresValues)
{
    const Circle value{Vector2{2.0F, 3.0F}, 4.0F};

    EXPECT_TRUE(value.center == (Vector2{2.0F, 3.0F}));
    EXPECT_FLOAT_EQ(value.radius, 4.0F);
}

TEST(CircleTests, Constructor_NegativeRadius_ClampsToZero)
{
    const Circle value{Vector2{2.0F, 3.0F}, -4.0F};

    EXPECT_FLOAT_EQ(value.radius, 0.0F);
}

TEST(CircleTests, Contains_PointInside_ReturnsTrue)
{
    const Circle value{Vector2{0.0F, 0.0F}, 5.0F};

    EXPECT_TRUE(value.contains(Vector2{3.0F, 4.0F}));
}

TEST(CircleTests, Contains_PointOnBoundary_ReturnsTrue)
{
    const Circle value{Vector2{0.0F, 0.0F}, 5.0F};

    EXPECT_TRUE(value.contains(Vector2{5.0F, 0.0F}));
}

TEST(CircleTests, Contains_PointOutside_ReturnsFalse)
{
    const Circle value{Vector2{0.0F, 0.0F}, 5.0F};

    EXPECT_FALSE(value.contains(Vector2{5.1F, 0.0F}));
}

TEST(CircleTests, Contains_CircleInside_ReturnsTrue)
{
    const Circle outer{Vector2{0.0F, 0.0F}, 10.0F};
    const Circle inner{Vector2{1.0F, 1.0F}, 2.0F};

    EXPECT_TRUE(outer.contains(inner));
}

TEST(CircleTests, Contains_CircleTouchingBoundary_ReturnsTrue)
{
    const Circle outer{Vector2{0.0F, 0.0F}, 10.0F};
    const Circle inner{Vector2{8.0F, 0.0F}, 2.0F};

    EXPECT_TRUE(outer.contains(inner));
}

TEST(CircleTests, Contains_LargerCircle_ReturnsFalse)
{
    const Circle inner{Vector2{0.0F, 0.0F}, 10.0F};
    const Circle outer{Vector2{1.0F, 0.0F}, 20.0F};

    EXPECT_FALSE(inner.contains(outer));
}

TEST(CircleTests, Intersects_OverlappingCircles_ReturnsTrue)
{
    const Circle first{Vector2{0.0F, 0.0F}, 5.0F};
    const Circle second{Vector2{6.0F, 0.0F}, 5.0F};

    EXPECT_TRUE(first.intersects(second));
}

TEST(CircleTests, Intersects_TouchingCircles_ReturnsTrue)
{
    const Circle first{Vector2{0.0F, 0.0F}, 5.0F};
    const Circle second{Vector2{10.0F, 0.0F}, 5.0F};

    EXPECT_TRUE(first.intersects(second));
}

TEST(CircleTests, Intersects_SeparatedCircles_ReturnsFalse)
{
    const Circle first{Vector2{0.0F, 0.0F}, 5.0F};
    const Circle second{Vector2{10.1F, 0.0F}, 5.0F};

    EXPECT_FALSE(first.intersects(second));
}

TEST(CircleTests, Intersects_ConcentricCircles_ReturnsTrue)
{
    const Circle outer{Vector2{0.0F, 0.0F}, 10.0F};
    const Circle inner{Vector2{0.0F, 0.0F}, 2.0F};

    EXPECT_TRUE(outer.intersects(inner));
}

TEST(CircleTests, Bounds_CenterAndRadius_ReturnsEnclosingRect)
{
    const Circle value{Vector2{2.0F, 3.0F}, 4.0F};

    const Rect result = value.bounds();

    EXPECT_TRUE(result.position == (Vector2{-2.0F, -1.0F}));
    EXPECT_TRUE(result.size == (Vector2{8.0F, 8.0F}));
}

TEST(CircleTests, Expanded_PositiveAmount_GrowsRadius)
{
    const Circle value{Vector2{2.0F, 3.0F}, 4.0F};

    const Circle result = value.expanded(2.0F);

    EXPECT_TRUE(result.center == (Vector2{2.0F, 3.0F}));
    EXPECT_FLOAT_EQ(result.radius, 6.0F);
}

TEST(CircleTests, Expanded_ShrinkBeyondRadius_ClampsToZero)
{
    const Circle value{Vector2{2.0F, 3.0F}, 4.0F};

    const Circle result = value.expanded(-10.0F);

    EXPECT_FLOAT_EQ(result.radius, 0.0F);
}

TEST(CircleTests, Equality_SameValues_ReturnsTrue)
{
    const Circle first{Vector2{1.0F, 2.0F}, 3.0F};
    const Circle second{Vector2{1.0F, 2.0F}, 3.0F};

    EXPECT_TRUE(first == second);
}

}  // namespace
