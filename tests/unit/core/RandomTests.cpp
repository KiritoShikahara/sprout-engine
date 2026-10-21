#include "sprout/core/Random.hpp"

#include <gtest/gtest.h>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

namespace {

using sprout::Circle;
using sprout::Random;
using sprout::Vector2;

TEST(RandomTests, SameSeed_ProducesSameSequence)
{
    Random first{12345};
    Random second{12345};

    for (int iteration = 0; iteration < 64; ++iteration) {
        EXPECT_EQ(first.nextUint64(), second.nextUint64());
        EXPECT_FLOAT_EQ(first.nextFloat(), second.nextFloat());
        EXPECT_EQ(first.nextInt(-10, 10), second.nextInt(-10, 10));
    }
}

TEST(RandomTests, DifferentSeeds_ProduceDifferentSequences)
{
    Random first{1};
    Random second{2};

    bool different = false;
    for (int iteration = 0; iteration < 32; ++iteration) {
        if (first.nextUint64() != second.nextUint64()) {
            different = true;
        }
    }

    EXPECT_TRUE(different);
}

TEST(RandomTests, SetSeed_RestartsSequence)
{
    Random random{999};
    const std::array<std::uint64_t, 8> expected = {
        random.nextUint64(),
        random.nextUint64(),
        random.nextUint64(),
        random.nextUint64(),
        random.nextUint64(),
        random.nextUint64(),
        random.nextUint64(),
        random.nextUint64(),
    };

    random.setSeed(999);

    EXPECT_EQ(random.seed(), 999U);
    for (const std::uint64_t value : expected) {
        EXPECT_EQ(random.nextUint64(), value);
    }
}

TEST(RandomTests, Seed_ReturnsConstructorValue)
{
    const Random random{0x0123456789ABCDEFULL};

    EXPECT_EQ(random.seed(), 0x0123456789ABCDEFULL);
}

TEST(RandomTests, NextInt_StaysWithinInclusiveRange)
{
    Random random{7};

    for (int iteration = 0; iteration < 2000; ++iteration) {
        const int value = random.nextInt(-3, 7);

        EXPECT_GE(value, -3);
        EXPECT_LE(value, 7);
    }
}

TEST(RandomTests, NextInt_EqualBounds_ReturnsBound)
{
    Random random{11};

    for (int iteration = 0; iteration < 16; ++iteration) {
        EXPECT_EQ(random.nextInt(5, 5), 5);
    }
}

TEST(RandomTests, NextInt_ReversedBounds_AreSwapped)
{
    Random random{13};

    for (int iteration = 0; iteration < 500; ++iteration) {
        const int value = random.nextInt(8, -2);

        EXPECT_GE(value, -2);
        EXPECT_LE(value, 8);
    }
}

TEST(RandomTests, NextInt_CoversEveryValue)
{
    Random random{17};
    std::array<bool, 6> seen{};

    for (int iteration = 0; iteration < 1000; ++iteration) {
        const int value = random.nextInt(1, 6);
        ASSERT_GE(value, 1);
        ASSERT_LE(value, 6);
        seen[static_cast<std::size_t>(value - 1)] = true;
    }

    for (const bool hit : seen) {
        EXPECT_TRUE(hit);
    }
}

TEST(RandomTests, NextFloat_StaysWithinUnitInterval)
{
    Random random{23};

    for (int iteration = 0; iteration < 10000; ++iteration) {
        const float value = random.nextFloat();

        EXPECT_GE(value, 0.0F);
        EXPECT_LT(value, 1.0F);
    }
}

TEST(RandomTests, Range_MaximumOnly_StaysBelowMaximum)
{
    Random random{29};

    for (int iteration = 0; iteration < 10000; ++iteration) {
        const float value = random.range(4.5F);

        EXPECT_GE(value, 0.0F);
        EXPECT_LT(value, 4.5F);
    }
}

TEST(RandomTests, Range_MinimumAndMaximum_StaysWithinInterval)
{
    Random random{31};

    for (int iteration = 0; iteration < 10000; ++iteration) {
        const float value = random.range(-2.5F, 7.5F);

        EXPECT_GE(value, -2.5F);
        EXPECT_LT(value, 7.5F);
    }
}

TEST(RandomTests, Range_EqualBounds_ReturnsBound)
{
    Random random{37};

    EXPECT_FLOAT_EQ(random.range(3.25F, 3.25F), 3.25F);
}

TEST(RandomTests, Range_ReversedBounds_AreSwapped)
{
    Random random{41};

    for (int iteration = 0; iteration < 1000; ++iteration) {
        const float value = random.range(9.0F, 1.0F);

        EXPECT_GE(value, 1.0F);
        EXPECT_LT(value, 9.0F);
    }
}

TEST(RandomTests, Chance_ZeroOrLessIsAlwaysFalse)
{
    Random random{43};

    for (int iteration = 0; iteration < 1000; ++iteration) {
        EXPECT_FALSE(random.chance(0.0F));
        EXPECT_FALSE(random.chance(-0.5F));
    }
}

TEST(RandomTests, Chance_OneOrMoreIsAlwaysTrue)
{
    Random random{47};

    for (int iteration = 0; iteration < 1000; ++iteration) {
        EXPECT_TRUE(random.chance(1.0F));
        EXPECT_TRUE(random.chance(1.5F));
    }
}

TEST(RandomTests, Chance_Half_IsRoughlyHalfTrue)
{
    Random random{53};
    int hits = 0;
    constexpr int kSamples = 10000;

    for (int iteration = 0; iteration < kSamples; ++iteration) {
        if (random.chance(0.5F)) {
            ++hits;
        }
    }

    EXPECT_GT(hits, 4000);
    EXPECT_LT(hits, 6000);
}

TEST(RandomTests, WeightedIndex_EmptyOrNonPositiveWeights_ReturnsNullopt)
{
    Random random{59};
    const std::vector<float> empty;
    const std::vector<float> zeros{0.0F, 0.0F, 0.0F};
    const std::vector<float> negatives{-1.0F, -2.0F};

    EXPECT_EQ(random.weightedIndex(empty), std::nullopt);
    EXPECT_EQ(random.weightedIndex(zeros), std::nullopt);
    EXPECT_EQ(random.weightedIndex(negatives), std::nullopt);
}

TEST(RandomTests, WeightedIndex_SinglePositiveWeight_ReturnsItsIndex)
{
    Random random{61};
    const std::vector<float> weights{0.0F, 2.5F, 0.0F};

    for (int iteration = 0; iteration < 100; ++iteration) {
        EXPECT_EQ(random.weightedIndex(weights), 1U);
    }
}

TEST(RandomTests, WeightedIndex_ZeroWeightEntries_AreNeverSelected)
{
    Random random{67};
    const std::vector<float> weights{0.0F, -3.0F, 1.0F};

    for (int iteration = 0; iteration < 1000; ++iteration) {
        EXPECT_EQ(random.weightedIndex(weights), 2U);
    }
}

TEST(RandomTests, WeightedIndex_FollowsWeightRatio)
{
    Random random{71};
    const std::vector<float> weights{1.0F, 3.0F};
    std::array<int, 2> counts{};
    constexpr int kSamples = 20000;

    for (int iteration = 0; iteration < kSamples; ++iteration) {
        const std::optional<std::size_t> index = random.weightedIndex(weights);
        ASSERT_TRUE(index.has_value());
        ASSERT_LT(*index, counts.size());
        ++counts[*index];
    }

    const double ratio = static_cast<double>(counts[0]) / kSamples;
    EXPECT_NEAR(ratio, 0.25, 0.03);
}

TEST(RandomTests, ChooseWeighted_ReturnsSelectedElement)
{
    Random random{73};
    constexpr std::array<std::string_view, 3> kItems{"sword", "shield", "potion"};
    const std::array<float, 3> weights{0.0F, 2.0F, 1.0F};

    for (int iteration = 0; iteration < 100; ++iteration) {
        const std::optional<std::string_view> item = random.chooseWeighted(
            std::span<const std::string_view>{kItems}, std::span<const float>{weights});

        ASSERT_TRUE(item.has_value());
        EXPECT_NE(*item, kItems[0]);
    }
}

TEST(RandomTests, ChooseWeighted_EmptyItems_ReturnsNullopt)
{
    Random random{79};
    constexpr std::span<const std::string_view> kItems{};
    const std::vector<float> weights{1.0F};
    const std::vector<std::string_view> items;
    const std::vector<float> emptyWeights;

    EXPECT_EQ(random.chooseWeighted(kItems, std::span<const float>{weights}), std::nullopt);
    EXPECT_EQ(
        random.chooseWeighted(std::span<const std::string_view>{items}, std::span<const float>{emptyWeights}),
        std::nullopt);
}

TEST(RandomTests, ChooseWeighted_LengthMismatch_ReturnsNullopt)
{
    Random random{83};
    const std::vector<std::string_view> items{"a", "b", "c"};
    const std::vector<float> weights{1.0F};
    const std::vector<float> longerWeights{1.0F, 1.0F, 1.0F, 1.0F};

    EXPECT_EQ(
        random.chooseWeighted(std::span<const std::string_view>{items}, std::span<const float>{weights}),
        std::nullopt);
    EXPECT_EQ(
        random.chooseWeighted(std::span<const std::string_view>{items}, std::span<const float>{longerWeights}),
        std::nullopt);
}

TEST(RandomTests, PointInCircle_StaysInsideCircle)
{
    Random random{89};
    const Circle circle{{5.0F, -2.0F}, 3.0F};
    const float limit = circle.radius * circle.radius + 1.0e-3F;

    for (int iteration = 0; iteration < 5000; ++iteration) {
        const Vector2 point = random.pointInCircle(circle);

        EXPECT_LE((point - circle.center).lengthSquared(), limit);
    }
}

TEST(RandomTests, PointInCircle_SameSeed_ProducesSamePoints)
{
    Random first{97};
    Random second{97};
    const Circle circle{{-4.0F, 8.0F}, 6.0F};

    for (int iteration = 0; iteration < 64; ++iteration) {
        const Vector2 a = first.pointInCircle(circle);
        const Vector2 b = second.pointInCircle(circle);

        EXPECT_FLOAT_EQ(a.x, b.x);
        EXPECT_FLOAT_EQ(a.y, b.y);
    }
}

TEST(RandomTests, PointInCircle_ZeroRadius_ReturnsCenter)
{
    Random random{101};
    const Circle circle{{1.5F, -3.5F}, 0.0F};

    const Vector2 point = random.pointInCircle(circle);

    EXPECT_FLOAT_EQ(point.x, circle.center.x);
    EXPECT_FLOAT_EQ(point.y, circle.center.y);
}

}  // namespace
