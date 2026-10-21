#include "sprout/math/Color.hpp"

#include <cstdint>
#include <gtest/gtest.h>

namespace {

using sprout::Color;

constexpr float kTolerance = 1.0e-4F;

void expectColorNear(const Color& actual, const Color& expected, int tolerance = 1)
{
    EXPECT_NEAR(actual.r, expected.r, tolerance);
    EXPECT_NEAR(actual.g, expected.g, tolerance);
    EXPECT_NEAR(actual.b, expected.b, tolerance);
    EXPECT_NEAR(actual.a, expected.a, tolerance);
}

}  // namespace

TEST(ColorTests, DefaultConstructor_NoArgs_IsOpaqueBlack)
{
    const Color color;

    EXPECT_EQ(color.r, 0);
    EXPECT_EQ(color.g, 0);
    EXPECT_EQ(color.b, 0);
    EXPECT_EQ(color.a, 255);
}

TEST(ColorTests, Constructor_Components_StoresValues)
{
    const Color color{17, 34, 51, 68};

    EXPECT_EQ(color.r, 17);
    EXPECT_EQ(color.g, 34);
    EXPECT_EQ(color.b, 51);
    EXPECT_EQ(color.a, 68);
}

TEST(ColorTests, Constructor_OmittedAlpha_IsOpaque)
{
    const Color color{1, 2, 3};

    EXPECT_EQ(color.a, 255);
}

TEST(ColorTests, Constants_PredefinedColors_ReturnExpectedValues)
{
    EXPECT_TRUE((Color::black() == Color{0, 0, 0, 255}));
    EXPECT_TRUE((Color::white() == Color{255, 255, 255, 255}));
    EXPECT_TRUE((Color::red() == Color{255, 0, 0, 255}));
    EXPECT_TRUE((Color::green() == Color{0, 255, 0, 255}));
    EXPECT_TRUE((Color::blue() == Color{0, 0, 255, 255}));
    EXPECT_TRUE((Color::yellow() == Color{255, 255, 0, 255}));
    EXPECT_TRUE((Color::magenta() == Color{255, 0, 255, 255}));
    EXPECT_TRUE((Color::cyan() == Color{0, 255, 255, 255}));
    EXPECT_TRUE((Color::transparent() == Color{0, 0, 0, 0}));
}

TEST(ColorTests, HexConstructor_PackedRgba_UnpacksComponents)
{
    const Color color{0x11223344U};

    EXPECT_EQ(color.r, 0x11);
    EXPECT_EQ(color.g, 0x22);
    EXPECT_EQ(color.b, 0x33);
    EXPECT_EQ(color.a, 0x44);
}

TEST(ColorTests, FromHex_PackedRgba_ReturnsSameColor)
{
    EXPECT_TRUE((Color::fromHex(0xABCDEF01U) == Color{0xAB, 0xCD, 0xEF, 0x01}));
}

TEST(ColorTests, ToHex_RoundTripsAllComponents)
{
    const Color color{0x12, 0x34, 0x56, 0x78};

    EXPECT_EQ(color.toHex(), 0x12345678U);
}

TEST(ColorTests, FromHsv_PrimaryColors_ReturnsExpectedRgb)
{
    const Color red = Color::fromHsv(0.0F, 1.0F, 1.0F);
    const Color green = Color::fromHsv(120.0F, 1.0F, 1.0F);
    const Color blue = Color::fromHsv(240.0F, 1.0F, 1.0F);

    EXPECT_TRUE(red == Color::red());
    EXPECT_TRUE(green == Color::green());
    EXPECT_TRUE(blue == Color::blue());
}

TEST(ColorTests, FromHsv_HueAboveRange_WrapsAround)
{
    EXPECT_TRUE(Color::fromHsv(360.0F, 1.0F, 1.0F) == Color::red());
    EXPECT_TRUE(Color::fromHsv(480.0F, 1.0F, 1.0F) == Color::green());
    EXPECT_TRUE(Color::fromHsv(-120.0F, 1.0F, 1.0F) == Color::blue());
}

TEST(ColorTests, FromHsv_Grayscale_ReturnsEqualComponents)
{
    const Color gray = Color::fromHsv(200.0F, 0.0F, 0.5F);
    const Color black = Color::fromHsv(0.0F, 0.0F, 0.0F);

    EXPECT_TRUE((gray == Color{128, 128, 128, 255}));
    EXPECT_TRUE((black == Color{0, 0, 0, 255}));
}

TEST(ColorTests, FromHsv_OutOfRangeComponents_ClampsToValidRange)
{
    const Color high = Color::fromHsv(0.0F, 2.0F, 2.0F, 2.0F);
    const Color low = Color::fromHsv(0.0F, -1.0F, -1.0F, -1.0F);

    EXPECT_TRUE((high == Color{255, 0, 0, 255}));
    EXPECT_TRUE((low == Color{0, 0, 0, 0}));
}

TEST(ColorTests, ToHsv_Red_ReturnsExpectedHsv)
{
    const Color::Hsv hsv = Color::red().toHsv();

    EXPECT_NEAR(hsv.hue, 0.0F, kTolerance);
    EXPECT_NEAR(hsv.saturation, 1.0F, kTolerance);
    EXPECT_NEAR(hsv.value, 1.0F, kTolerance);
    EXPECT_NEAR(hsv.alpha, 1.0F, kTolerance);
}

TEST(ColorTests, ToHsv_Black_ReturnsZeroSaturationAndValue)
{
    const Color::Hsv hsv = Color::black().toHsv();

    EXPECT_NEAR(hsv.hue, 0.0F, kTolerance);
    EXPECT_NEAR(hsv.saturation, 0.0F, kTolerance);
    EXPECT_NEAR(hsv.value, 0.0F, kTolerance);
    EXPECT_NEAR(hsv.alpha, 1.0F, kTolerance);
}

TEST(ColorTests, HsvRoundTrip_ArbitraryColor_RestoresComponents)
{
    const Color original{64, 128, 200, 128};

    expectColorNear(Color::fromHsv(original.toHsv()), original);
}

TEST(ColorTests, Lerp_Endpoints_ReturnsInputColors)
{
    const Color from{10, 20, 30, 40};
    const Color to{200, 210, 220, 230};

    EXPECT_TRUE(Color::lerp(from, to, 0.0F) == from);
    EXPECT_TRUE(Color::lerp(from, to, 1.0F) == to);
}

TEST(ColorTests, Lerp_Midpoint_InterpolatesEachComponent)
{
    const Color result = Color::lerp(Color::black(), Color::white(), 0.5F);

    EXPECT_TRUE((result == Color{128, 128, 128, 255}));
}

TEST(ColorTests, Lerp_OutOfRangeFactor_ClampsToEndpoints)
{
    const Color from{10, 20, 30, 40};
    const Color to{200, 210, 220, 230};

    EXPECT_TRUE(Color::lerp(from, to, -1.0F) == from);
    EXPECT_TRUE(Color::lerp(from, to, 2.0F) == to);
}

TEST(ColorTests, Lerp_TransparentTarget_InterpolatesAlpha)
{
    const Color result = Color::lerp(Color{255, 0, 0, 255}, Color{255, 0, 0, 0}, 0.5F);

    EXPECT_TRUE((result == Color{255, 0, 0, 128}));
}

TEST(ColorTests, Equality_SameComponents_IsTrue)
{
    EXPECT_TRUE((Color{1, 2, 3, 4} == Color{1, 2, 3, 4}));
}

TEST(ColorTests, Equality_DifferentComponents_IsFalse)
{
    EXPECT_FALSE((Color{1, 2, 3, 4} == Color{1, 2, 3, 5}));
    EXPECT_TRUE((Color{1, 2, 3, 4} != Color{1, 2, 3, 5}));
}
