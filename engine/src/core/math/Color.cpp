#include "sprout/math/Color.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace {

std::uint8_t toByte(float normalized) noexcept
{
    const float clamped = std::clamp(normalized, 0.0F, 1.0F);
    return static_cast<std::uint8_t>(std::lround(clamped * 255.0F));
}

float normalizeHue(float hue) noexcept
{
    const float wrapped = std::fmod(hue, 360.0F);
    return wrapped < 0.0F ? wrapped + 360.0F : wrapped;
}

}  // namespace

namespace sprout {

Color Color::fromHsv(const Hsv& hsv) noexcept
{
    return fromHsv(hsv.hue, hsv.saturation, hsv.value, hsv.alpha);
}

Color Color::fromHsv(float hue, float saturation, float value, float alpha) noexcept
{
    const float normalizedHue = normalizeHue(hue);
    const float clampedSaturation = std::clamp(saturation, 0.0F, 1.0F);
    const float clampedValue = std::clamp(value, 0.0F, 1.0F);
    const float clampedAlpha = std::clamp(alpha, 0.0F, 1.0F);

    const float chroma = clampedValue * clampedSaturation;
    const float hueSection = normalizedHue / 60.0F;
    const float secondComponent = chroma * (1.0F - std::abs(std::fmod(hueSection, 2.0F) - 1.0F));
    const float offset = clampedValue - chroma;

    float red = 0.0F;
    float green = 0.0F;
    float blue = 0.0F;
    if (hueSection < 1.0F) {
        red = chroma;
        green = secondComponent;
    } else if (hueSection < 2.0F) {
        red = secondComponent;
        green = chroma;
    } else if (hueSection < 3.0F) {
        green = chroma;
        blue = secondComponent;
    } else if (hueSection < 4.0F) {
        green = secondComponent;
        blue = chroma;
    } else if (hueSection < 5.0F) {
        red = secondComponent;
        blue = chroma;
    } else {
        red = chroma;
        blue = secondComponent;
    }

    return {toByte(red + offset), toByte(green + offset), toByte(blue + offset), toByte(clampedAlpha)};
}

Color::Hsv Color::toHsv() const noexcept
{
    const float red = static_cast<float>(r) / 255.0F;
    const float green = static_cast<float>(g) / 255.0F;
    const float blue = static_cast<float>(b) / 255.0F;

    const float maximum = std::max({red, green, blue});
    const float minimum = std::min({red, green, blue});
    const float delta = maximum - minimum;

    Hsv result{};
    result.value = maximum;
    result.alpha = static_cast<float>(a) / 255.0F;
    if (delta > 0.0F) {
        result.saturation = delta / maximum;
        if (maximum == red) {
            result.hue = 60.0F * std::fmod((green - blue) / delta, 6.0F);
        } else if (maximum == green) {
            result.hue = 60.0F * ((blue - red) / delta + 2.0F);
        } else {
            result.hue = 60.0F * ((red - green) / delta + 4.0F);
        }
        if (result.hue < 0.0F) {
            result.hue += 360.0F;
        }
    }
    return result;
}

Color Color::lerp(const Color& from, const Color& to, float t) noexcept
{
    const float clamped = std::clamp(t, 0.0F, 1.0F);

    const auto interpolate = [clamped](std::uint8_t start, std::uint8_t end) {
        return static_cast<std::uint8_t>(
            std::lround(static_cast<float>(start) + (static_cast<float>(end) - static_cast<float>(start)) * clamped));
    };

    return {interpolate(from.r, to.r), interpolate(from.g, to.g), interpolate(from.b, to.b),
            interpolate(from.a, to.a)};
}

}  // namespace sprout
