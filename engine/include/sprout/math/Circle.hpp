#pragma once

#include "sprout/math/Rect.hpp"
#include "sprout/math/Vector2.hpp"

#include <algorithm>

namespace sprout {

/// @brief 円。
///
/// 中心と半径で表す。境界上の点・円は含むものとする。
///
/// @code
/// const sprout::Circle circle{{0.0F, 0.0F}, 10.0F};
/// const bool hit = circle.contains(sprout::Vector2{3.0F, 4.0F}); // true
/// const sprout::Rect box = circle.bounds();
/// @endcode
struct Circle {
    /// 中心座標。
    Vector2 center{};

    /// 半径。負の値はゼロに丸められる。
    float radius{0.0F};

    /// 中心が原点で半径がゼロの円を生成する。
    constexpr Circle() noexcept = default;

    /// 中心と半径を指定して生成する。
    /// @param centerValue 中心座標。
    /// @param radiusValue 半径。負の値はゼロとして扱う。
    constexpr Circle(const Vector2& centerValue, float radiusValue) noexcept
        : center{centerValue}
        , radius{std::max(0.0F, radiusValue)}
    {
    }

    /// 点を含むか判定する。境界上の点は含む。
    /// @param point 判定する点。
    /// @return 円の内側または境界上なら true。
    constexpr bool contains(const Vector2& point) const noexcept
    {
        return (point - center).lengthSquared() <= radius * radius;
    }

    /// 別の円を完全に含むか判定する。内接する場合は含む。
    /// @param other 判定する円。
    /// @return other が自身の内側または境界上に収まれば true。
    constexpr bool contains(const Circle& other) const noexcept
    {
        if (other.radius > radius) {
            return false;
        }
        const float difference = radius - other.radius;
        return (other.center - center).lengthSquared() <= difference * difference;
    }

    /// 別の円と交差するか判定する。外接・内接する場合は交差とみなす。
    /// @param other 判定する円。
    /// @return 重なりまたは接触があれば true。
    constexpr bool intersects(const Circle& other) const noexcept
    {
        const float distance = radius + other.radius;
        return (other.center - center).lengthSquared() <= distance * distance;
    }

    /// 円を包含する軸平行矩形を返す。
    /// @return 外接する矩形。
    constexpr Rect bounds() const noexcept
    {
        const Vector2 extent{radius, radius};
        return Rect{center - extent, extent * 2.0F};
    }

    /// 半径を拡張（負の値なら縮小）した円を返す。中心は変わらない。
    /// @param amount 半径に加える量。負の場合は内側へ縮める。
    /// @return 拡張後の円。半径が負になる場合はゼロに丸める。
    constexpr Circle expanded(float amount) const noexcept
    {
        return Circle{center, std::max(0.0F, radius + amount)};
    }

    /// 全成分が完全一致するか判定する。
    /// @param other 比較対象。
    /// @return 一致すれば true。
    constexpr bool operator==(const Circle& other) const noexcept = default;
};

}  // namespace sprout
