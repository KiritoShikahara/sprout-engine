#pragma once

#include "sprout/math/Vector2.hpp"

#include <array>
#include <optional>

namespace sprout {

/// @brief 2D アフィン変換を表す 3x3 行列。
///
/// 点の変換は列ベクトル規約で行う。`lhs * rhs` は「rhs を適用してから lhs を適用する」合成を表す。
/// 最下行は常に (0, 0, 1) である。
///
/// @code
/// const sprout::Matrix3 transform =
///     sprout::Matrix3::translation({10.0F, 0.0F}) * sprout::Matrix3::rotation(0.5F) *
///     sprout::Matrix3::scale({2.0F, 2.0F});
/// const sprout::Vector2 world = transform.transformPoint({1.0F, 2.0F});
/// const sprout::Vector2 forward = transform.transformDirection(sprout::Vector2::right());
/// @endcode
class Matrix3 {
public:
    /// 行列式がこの値以下の場合は逆行列を持たないとみなす。
    static constexpr float kMinimumDeterminant = 1.0e-9F;

    /// 単位行列を生成する。
    constexpr Matrix3() noexcept
        : m_values{1.0F, 0.0F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F, 0.0F, 1.0F}
    {
    }

    /// 単位行列を返す。
    /// @return 単位行列。
    static constexpr Matrix3 identity() noexcept
    {
        return {};
    }

    /// 平行移動行列を生成する。
    /// @param offset 移動量。
    /// @return 平行移動行列。
    static constexpr Matrix3 translation(const Vector2& offset) noexcept
    {
        return Matrix3{1.0F, 0.0F, offset.x, 0.0F, 1.0F, offset.y};
    }

    /// 回転行列を生成する。
    /// @param radians 反時計回りの回転角（ラジアン）。
    /// @return 回転行列。
    static Matrix3 rotation(float radians) noexcept;

    /// 成分ごとの拡大縮小行列を生成する。
    /// @param factor X 成分と Y 成分の倍率。
    /// @return 拡大縮小行列。
    static constexpr Matrix3 scale(const Vector2& factor) noexcept
    {
        return Matrix3{factor.x, 0.0F, 0.0F, 0.0F, factor.y, 0.0F};
    }

    /// 等方拡大縮小行列を生成する。
    /// @param factor 全成分に掛ける倍率。
    /// @return 拡大縮小行列。
    static constexpr Matrix3 scale(float factor) noexcept
    {
        return scale({factor, factor});
    }

    /// 点を変換する。平行移動も適用する。
    /// @param point 変換する点。
    /// @return 変換後の点。
    Vector2 transformPoint(const Vector2& point) const noexcept;

    /// 方向ベクトルを変換する。平行移動は適用しない。
    /// @param direction 変換する方向。
    /// @return 変換後の方向。
    Vector2 transformDirection(const Vector2& direction) const noexcept;

    /// 行列式を返す。
    /// @return 2x2 線形成分の行列式。
    float determinant() const noexcept;

    /// 逆行列を返す。
    /// @return 逆行列。行列式が kMinimumDeterminant 以下の場合は std::nullopt。
    std::optional<Matrix3> inverse() const noexcept;

    /// 行列を合成する。右辺を先に適用する。
    /// @param rhs 先に適用する行列。
    /// @return 合成した行列。
    Matrix3 operator*(const Matrix3& rhs) const noexcept;

    /// 全成分が完全一致するか判定する。
    /// @param rhs 比較対象。
    /// @return 一致すれば true。
    bool operator==(const Matrix3& rhs) const noexcept = default;

private:
    constexpr Matrix3(float value00, float value01, float value02, float value10, float value11, float value12) noexcept
        : m_values{value00, value01, value02, value10, value11, value12, 0.0F, 0.0F, 1.0F}
    {
    }

    constexpr explicit Matrix3(const std::array<float, 9>& values) noexcept
        : m_values{values}
    {
    }

    std::array<float, 9> m_values{};
};

}  // namespace sprout
