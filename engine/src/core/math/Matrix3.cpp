#include "sprout/math/Matrix3.hpp"

#include <cmath>
#include <cstddef>

namespace sprout {

Matrix3 Matrix3::rotation(float radians) noexcept
{
    const float cosine = std::cos(radians);
    const float sine = std::sin(radians);
    return Matrix3{cosine, -sine, 0.0F, sine, cosine, 0.0F};
}

Vector2 Matrix3::transformPoint(const Vector2& point) const noexcept
{
    return {m_values[0] * point.x + m_values[1] * point.y + m_values[2],
            m_values[3] * point.x + m_values[4] * point.y + m_values[5]};
}

Vector2 Matrix3::transformDirection(const Vector2& direction) const noexcept
{
    return {m_values[0] * direction.x + m_values[1] * direction.y,
            m_values[3] * direction.x + m_values[4] * direction.y};
}

float Matrix3::determinant() const noexcept
{
    return m_values[0] * m_values[4] - m_values[1] * m_values[3];
}

std::optional<Matrix3> Matrix3::inverse() const noexcept
{
    const float determinantValue = determinant();
    if (std::abs(determinantValue) <= kMinimumDeterminant) {
        return std::nullopt;
    }

    const float inverseDeterminant = 1.0F / determinantValue;
    const float value00 = m_values[4] * inverseDeterminant;
    const float value01 = -m_values[1] * inverseDeterminant;
    const float value10 = -m_values[3] * inverseDeterminant;
    const float value11 = m_values[0] * inverseDeterminant;
    const float value02 = (m_values[1] * m_values[5] - m_values[4] * m_values[2]) * inverseDeterminant;
    const float value12 = (m_values[3] * m_values[2] - m_values[0] * m_values[5]) * inverseDeterminant;
    return Matrix3{value00, value01, value02, value10, value11, value12};
}

Matrix3 Matrix3::operator*(const Matrix3& rhs) const noexcept
{
    constexpr std::size_t kSize = 3;
    std::array<float, 9> values{};
    for (std::size_t row = 0; row < kSize; ++row) {
        for (std::size_t column = 0; column < kSize; ++column) {
            float sum = 0.0F;
            for (std::size_t index = 0; index < kSize; ++index) {
                sum += m_values[row * kSize + index] * rhs.m_values[index * kSize + column];
            }
            values[row * kSize + column] = sum;
        }
    }
    return Matrix3{values};
}

}  // namespace sprout
