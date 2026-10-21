#include "sprout/core/Random.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace {

constexpr float kFloatUnit = 1.0F / 16777216.0F;

}  // namespace

namespace sprout {

Random::Random() noexcept
{
    std::random_device device;
    const std::uint64_t high = static_cast<std::uint64_t>(device()) << 32U;
    const std::uint64_t low = static_cast<std::uint64_t>(device());
    setSeed(high | low);
}

Random::Random(std::uint64_t seed) noexcept
{
    setSeed(seed);
}

void Random::setSeed(std::uint64_t seed) noexcept
{
    m_seed = seed;
    m_engine.seed(seed);
}

std::uint64_t Random::seed() const noexcept
{
    return m_seed;
}

std::uint64_t Random::nextUint64() noexcept
{
    return m_engine();
}

std::uint32_t Random::nextUint32() noexcept
{
    return static_cast<std::uint32_t>(m_engine() >> 32U);
}

int Random::nextInt(int minimum, int maximum) noexcept
{
    if (minimum > maximum) {
        std::swap(minimum, maximum);
    }
    std::uniform_int_distribution<int> distribution{minimum, maximum};
    return distribution(m_engine);
}

float Random::nextFloat() noexcept
{
    return static_cast<float>(nextUint32() >> 8U) * kFloatUnit;
}

float Random::range(float maximum) noexcept
{
    return maximum * nextFloat();
}

float Random::range(float minimum, float maximum) noexcept
{
    if (minimum > maximum) {
        std::swap(minimum, maximum);
    }
    return minimum + (maximum - minimum) * nextFloat();
}

bool Random::chance(float probability) noexcept
{
    if (probability <= 0.0F) {
        return false;
    }
    if (probability >= 1.0F) {
        return true;
    }
    return nextFloat() < probability;
}

std::optional<std::size_t> Random::weightedIndex(std::span<const float> weights) noexcept
{
    double total = 0.0;
    for (const float weight : weights) {
        if (std::isfinite(weight) && weight > 0.0F) {
            total += static_cast<double>(weight);
        }
    }
    if (total <= 0.0) {
        return std::nullopt;
    }

    const double target = static_cast<double>(nextFloat()) * total;
    double cumulative = 0.0;
    std::optional<std::size_t> lastValid;
    for (std::size_t index = 0; index < weights.size(); ++index) {
        const float weight = weights[index];
        if (!std::isfinite(weight) || weight <= 0.0F) {
            continue;
        }
        cumulative += static_cast<double>(weight);
        lastValid = index;
        if (target < cumulative) {
            return index;
        }
    }
    return lastValid;
}

Vector2 Random::pointInCircle(const Circle& circle) noexcept
{
    if (circle.radius <= 0.0F) {
        return circle.center;
    }
    const float distance = circle.radius * std::sqrt(nextFloat());
    const float angle = 2.0F * std::numbers::pi_v<float> * nextFloat();
    return {
        circle.center.x + distance * std::cos(angle),
        circle.center.y + distance * std::sin(angle),
    };
}

}  // namespace sprout
