#pragma once

#include "sprout/math/Vector2.hpp"

#include <algorithm>

namespace sprout {

/// @brief 軸に平行な矩形。
///
/// 左上を基準とする位置と大きさで表す。座標系の向きには依存せず、size は
/// 各軸の正方向へ広がる長さとして扱う。境界上の点・辺は含むものとする。
///
/// @code
/// const sprout::Rect rect{{0.0F, 0.0F}, {32.0F, 16.0F}};
/// const bool hit = rect.contains(sprout::Vector2{8.0F, 8.0F}); // true
/// const sprout::Rect grown = rect.expanded(4.0F);
/// @endcode
struct Rect {
    /// 最小隅の位置。
    Vector2 position{};

    /// 各軸方向の大きさ。負の値は想定しない。
    Vector2 size{};

    /// 位置と大きさがともにゼロの矩形を生成する。
    constexpr Rect() noexcept = default;

    /// 位置と大きさを指定して生成する。
    /// @param positionValue 最小隅の位置。
    /// @param sizeValue 各軸方向の大きさ。
    constexpr Rect(const Vector2& positionValue, const Vector2& sizeValue) noexcept
        : position{positionValue}
        , size{sizeValue}
    {
    }

    /// 成分を指定して生成する。
    /// @param x 最小隅の X 座標。
    /// @param y 最小隅の Y 座標。
    /// @param width X 軸方向の大きさ。
    /// @param height Y 軸方向の大きさ。
    constexpr Rect(float x, float y, float width, float height) noexcept
        : position{x, y}
        , size{width, height}
    {
    }

    /// 最小隅を返す。
    /// @return position と同じベクトル。
    constexpr Vector2 minimum() const noexcept
    {
        return position;
    }

    /// 最大隅を返す。
    /// @return position + size。
    constexpr Vector2 maximum() const noexcept
    {
        return position + size;
    }

    /// 中心を返す。
    /// @return 矩形の中心座標。
    constexpr Vector2 center() const noexcept
    {
        return position + size * 0.5F;
    }

    /// 点を含むか判定する。境界上の点は含む。
    /// @param point 判定する点。
    /// @return 矩形の内側または境界上なら true。
    constexpr bool contains(const Vector2& point) const noexcept
    {
        const Vector2 farCorner = maximum();
        return point.x >= position.x && point.x <= farCorner.x && point.y >= position.y && point.y <= farCorner.y;
    }

    /// 別の矩形を完全に含むか判定する。境界が一致する場合は含む。
    /// @param other 判定する矩形。
    /// @return other が自身の内側または境界上に収まれば true。
    constexpr bool contains(const Rect& other) const noexcept
    {
        const Vector2 farCorner = maximum();
        const Vector2 otherFarCorner = other.maximum();
        return other.position.x >= position.x && other.position.y >= position.y && otherFarCorner.x <= farCorner.x
               && otherFarCorner.y <= farCorner.y;
    }

    /// 別の矩形と交差するか判定する。辺や頂点で接する場合は交差とみなす。
    /// @param other 判定する矩形。
    /// @return 重なりまたは接触があれば true。
    constexpr bool intersects(const Rect& other) const noexcept
    {
        const Vector2 farCorner = maximum();
        const Vector2 otherFarCorner = other.maximum();
        return position.x <= otherFarCorner.x && other.position.x <= farCorner.x && position.y <= otherFarCorner.y
               && other.position.y <= farCorner.y;
    }

    /// 2 つの矩形を包含する最小の矩形を返す。
    /// @param other 合成する矩形。
    /// @return 両方を包含する矩形。
    constexpr Rect united(const Rect& other) const noexcept
    {
        const Vector2 nearCorner{std::min(position.x, other.position.x), std::min(position.y, other.position.y)};
        const Vector2 farCorner = maximum();
        const Vector2 otherFarCorner = other.maximum();
        return Rect{nearCorner,
                    {std::max(farCorner.x, otherFarCorner.x) - nearCorner.x,
                     std::max(farCorner.y, otherFarCorner.y) - nearCorner.y}};
    }

    /// 全方向に拡張（負の値なら縮小）した矩形を返す。中心は変わらない。
    /// @param amount 各辺を外側へ広げる量。負の場合は内側へ縮める。
    /// @return 拡張後の矩形。大きさが負になる場合はゼロに丸める。
    constexpr Rect expanded(float amount) const noexcept
    {
        const Vector2 newSize{std::max(0.0F, size.x + amount * 2.0F), std::max(0.0F, size.y + amount * 2.0F)};
        return Rect{center() - newSize * 0.5F, newSize};
    }

    /// 全成分が完全一致するか判定する。
    /// @param other 比較対象。
    /// @return 一致すれば true。
    constexpr bool operator==(const Rect& other) const noexcept = default;
};

}  // namespace sprout
