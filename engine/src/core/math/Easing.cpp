#include "sprout/math/Easing.hpp"

#include <algorithm>
#include <cmath>

namespace sprout {

namespace {

constexpr float kPi = 3.14159265358979323846F;
constexpr float kBackOvershoot = 1.70158F;
constexpr float kBackOvershootInOut = 2.5949095F;
constexpr float kElasticFrequency = 2.0943951023931953F;
constexpr float kElasticFrequencyInOut = 1.3962634015954636F;
constexpr float kBounceScale = 7.5625F;
constexpr float kBounceDivisor = 2.75F;

float clampUnit(float t) noexcept
{
    return std::clamp(t, 0.0F, 1.0F);
}

float bounceOut(float t) noexcept
{
    if (t < 1.0F / kBounceDivisor) {
        return kBounceScale * t * t;
    }
    if (t < 2.0F / kBounceDivisor) {
        const float shifted = t - 1.5F / kBounceDivisor;
        return kBounceScale * shifted * shifted + 0.75F;
    }
    if (t < 2.5F / kBounceDivisor) {
        const float shifted = t - 2.25F / kBounceDivisor;
        return kBounceScale * shifted * shifted + 0.9375F;
    }
    const float shifted = t - 2.625F / kBounceDivisor;
    return kBounceScale * shifted * shifted + 0.984375F;
}

}  // namespace

float easeInSine(float t) noexcept
{
    const float x = clampUnit(t);
    return 1.0F - std::cos(x * kPi * 0.5F);
}

float easeOutSine(float t) noexcept
{
    const float x = clampUnit(t);
    return std::sin(x * kPi * 0.5F);
}

float easeInOutSine(float t) noexcept
{
    const float x = clampUnit(t);
    return (1.0F - std::cos(kPi * x)) * 0.5F;
}

float easeInQuad(float t) noexcept
{
    const float x = clampUnit(t);
    return x * x;
}

float easeOutQuad(float t) noexcept
{
    const float x = clampUnit(t);
    return 1.0F - (1.0F - x) * (1.0F - x);
}

float easeInOutQuad(float t) noexcept
{
    const float x = clampUnit(t);
    if (x < 0.5F) {
        return 2.0F * x * x;
    }
    const float shifted = -2.0F * x + 2.0F;
    return 1.0F - shifted * shifted * 0.5F;
}

float easeInCubic(float t) noexcept
{
    const float x = clampUnit(t);
    return x * x * x;
}

float easeOutCubic(float t) noexcept
{
    const float x = clampUnit(t);
    const float shifted = 1.0F - x;
    return 1.0F - shifted * shifted * shifted;
}

float easeInOutCubic(float t) noexcept
{
    const float x = clampUnit(t);
    if (x < 0.5F) {
        return 4.0F * x * x * x;
    }
    const float shifted = -2.0F * x + 2.0F;
    return 1.0F - shifted * shifted * shifted * 0.5F;
}

float easeInQuart(float t) noexcept
{
    const float x = clampUnit(t);
    return x * x * x * x;
}

float easeOutQuart(float t) noexcept
{
    const float x = clampUnit(t);
    const float shifted = 1.0F - x;
    const float squared = shifted * shifted;
    return 1.0F - squared * squared;
}

float easeInOutQuart(float t) noexcept
{
    const float x = clampUnit(t);
    if (x < 0.5F) {
        return 8.0F * x * x * x * x;
    }
    const float shifted = -2.0F * x + 2.0F;
    const float squared = shifted * shifted;
    return 1.0F - squared * squared * 0.5F;
}

float easeInQuint(float t) noexcept
{
    const float x = clampUnit(t);
    return x * x * x * x * x;
}

float easeOutQuint(float t) noexcept
{
    const float x = clampUnit(t);
    const float shifted = 1.0F - x;
    const float squared = shifted * shifted;
    return 1.0F - squared * squared * shifted;
}

float easeInOutQuint(float t) noexcept
{
    const float x = clampUnit(t);
    if (x < 0.5F) {
        return 16.0F * x * x * x * x * x;
    }
    const float shifted = -2.0F * x + 2.0F;
    const float squared = shifted * shifted;
    return 1.0F - squared * squared * shifted * 0.5F;
}

float easeInExpo(float t) noexcept
{
    const float x = clampUnit(t);
    if (x == 0.0F) {
        return 0.0F;
    }
    return std::pow(2.0F, 10.0F * x - 10.0F);
}

float easeOutExpo(float t) noexcept
{
    const float x = clampUnit(t);
    if (x == 1.0F) {
        return 1.0F;
    }
    return 1.0F - std::pow(2.0F, -10.0F * x);
}

float easeInOutExpo(float t) noexcept
{
    const float x = clampUnit(t);
    if (x == 0.0F || x == 1.0F) {
        return x;
    }
    if (x < 0.5F) {
        return std::pow(2.0F, 20.0F * x - 10.0F) * 0.5F;
    }
    return (2.0F - std::pow(2.0F, -20.0F * x + 10.0F)) * 0.5F;
}

float easeInCirc(float t) noexcept
{
    const float x = clampUnit(t);
    return 1.0F - std::sqrt(1.0F - x * x);
}

float easeOutCirc(float t) noexcept
{
    const float x = clampUnit(t);
    const float shifted = x - 1.0F;
    return std::sqrt(1.0F - shifted * shifted);
}

float easeInOutCirc(float t) noexcept
{
    const float x = clampUnit(t);
    if (x < 0.5F) {
        const float doubled = 2.0F * x;
        return (1.0F - std::sqrt(1.0F - doubled * doubled)) * 0.5F;
    }
    const float shifted = -2.0F * x + 2.0F;
    return (std::sqrt(1.0F - shifted * shifted) + 1.0F) * 0.5F;
}

float easeInBack(float t) noexcept
{
    const float x = clampUnit(t);
    return (kBackOvershoot + 1.0F) * x * x * x - kBackOvershoot * x * x;
}

float easeOutBack(float t) noexcept
{
    const float x = clampUnit(t);
    const float shifted = x - 1.0F;
    return 1.0F + (kBackOvershoot + 1.0F) * shifted * shifted * shifted + kBackOvershoot * shifted * shifted;
}

float easeInOutBack(float t) noexcept
{
    const float x = clampUnit(t);
    if (x < 0.5F) {
        const float doubled = 2.0F * x;
        return (doubled * doubled * ((kBackOvershootInOut + 1.0F) * doubled - kBackOvershootInOut)) * 0.5F;
    }
    const float shifted = 2.0F * x - 2.0F;
    return (shifted * shifted * ((kBackOvershootInOut + 1.0F) * shifted + kBackOvershootInOut) + 2.0F) * 0.5F;
}

float easeInElastic(float t) noexcept
{
    const float x = clampUnit(t);
    if (x == 0.0F || x == 1.0F) {
        return x;
    }
    return -std::pow(2.0F, 10.0F * x - 10.0F) * std::sin((10.0F * x - 10.75F) * kElasticFrequency);
}

float easeOutElastic(float t) noexcept
{
    const float x = clampUnit(t);
    if (x == 0.0F || x == 1.0F) {
        return x;
    }
    return std::pow(2.0F, -10.0F * x) * std::sin((10.0F * x - 0.75F) * kElasticFrequency) + 1.0F;
}

float easeInOutElastic(float t) noexcept
{
    const float x = clampUnit(t);
    if (x == 0.0F || x == 1.0F) {
        return x;
    }
    if (x < 0.5F) {
        return -(std::pow(2.0F, 20.0F * x - 10.0F) * std::sin((20.0F * x - 11.125F) * kElasticFrequencyInOut)) *
            0.5F;
    }
    return std::pow(2.0F, -20.0F * x + 10.0F) * std::sin((20.0F * x - 11.125F) * kElasticFrequencyInOut) * 0.5F +
        1.0F;
}

float easeInBounce(float t) noexcept
{
    const float x = clampUnit(t);
    return 1.0F - bounceOut(1.0F - x);
}

float easeOutBounce(float t) noexcept
{
    return bounceOut(clampUnit(t));
}

float easeInOutBounce(float t) noexcept
{
    const float x = clampUnit(t);
    if (x < 0.5F) {
        return (1.0F - bounceOut(1.0F - 2.0F * x)) * 0.5F;
    }
    return (1.0F + bounceOut(2.0F * x - 1.0F)) * 0.5F;
}

}  // namespace sprout
