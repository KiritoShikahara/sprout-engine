#pragma once

#include <cmath>
#include <limits>
#include <numbers>

namespace sprout {

struct Degrees;

/// @brief ラジアン角（強い型）。
///
/// 生の float と取り違えないよう、生成には明示的な指定を必要とする。
///
/// @code
/// const sprout::Radians halfTurn{std::numbers::pi_v<float>};
/// const sprout::Degrees degrees = halfTurn.toDegrees(); // 180
/// @endcode
struct Radians {
    /// 角度（ラジアン）。
    float value{0.0F};

    /// 0 ラジアンを生成する。
    constexpr Radians() noexcept = default;

    /// 値を指定して生成する。
    /// @param radians 角度（ラジアン）。
    constexpr explicit Radians(float radians) noexcept
        : value{radians}
    {
    }

    /// 度に変換する。
    /// @return 等価な角度（度）。
    constexpr Degrees toDegrees() const noexcept;

    /// 同じ値か判定する。
    /// @param other 比較対象。
    /// @return 完全一致すれば true。
    constexpr bool operator==(const Radians& other) const noexcept = default;
};

/// @brief 度角（強い型）。
///
/// 生の float と取り違えないよう、生成には明示的な指定を必要とする。
///
/// @code
/// const sprout::Degrees rightAngle{90.0F};
/// const sprout::Radians radians = rightAngle.toRadians(); // π / 2
/// @endcode
struct Degrees {
    /// 角度（度）。
    float value{0.0F};

    /// 0 度を生成する。
    constexpr Degrees() noexcept = default;

    /// 値を指定して生成する。
    /// @param degrees 角度（度）。
    constexpr explicit Degrees(float degrees) noexcept
        : value{degrees}
    {
    }

    /// ラジアンに変換する。
    /// @return 等価な角度（ラジアン）。
    constexpr Radians toRadians() const noexcept
    {
        return Radians{static_cast<float>(static_cast<double>(value) * (std::numbers::pi_v<double> / 180.0))};
    }

    /// 同じ値か判定する。
    /// @param other 比較対象。
    /// @return 完全一致すれば true。
    constexpr bool operator==(const Degrees& other) const noexcept = default;
};

constexpr Degrees Radians::toDegrees() const noexcept
{
    return Degrees{static_cast<float>(static_cast<double>(value) * (180.0 / std::numbers::pi_v<double>))};
}

/// @brief 角度。内部表現はラジアン。
///
/// 度とラジアンの混在を防ぐため、生成は Degrees か Radians を明示して行う。
///
/// @code
/// const sprout::Angle halfTurn = sprout::Angle::fromDegrees(180.0F);
/// const float radians = halfTurn.radians(); // π
/// const sprout::Angle wrapped = sprout::Angle::fromDegrees(450.0F).wrapped();
/// @endcode
class Angle {
public:
    /// 半回転（π ラジアン）。
    static constexpr float kPi = std::numbers::pi_v<float>;

    /// 一回転（2π ラジアン）。
    static constexpr float kTwoPi = 2.0F * kPi;

    /// 0 ラジアンを生成する。
    constexpr Angle() noexcept = default;

    /// ラジアン角から生成する。
    /// @param radians 角度（ラジアン）。
    constexpr explicit Angle(Radians radians) noexcept
        : m_radians{radians}
    {
    }

    /// 度角から生成する。
    /// @param degrees 角度（度）。
    constexpr explicit Angle(Degrees degrees) noexcept
        : m_radians{degrees.toRadians()}
    {
    }

    /// ラジアン値を指定して生成する。
    /// @param radians 角度（ラジアン）。
    /// @return 生成した角度。
    static constexpr Angle fromRadians(float radians) noexcept
    {
        return Angle{Radians{radians}};
    }

    /// ラジアン角を指定して生成する。
    /// @param radians 角度（ラジアン）。
    /// @return 生成した角度。
    static constexpr Angle fromRadians(Radians radians) noexcept
    {
        return Angle{radians};
    }

    /// 度値を指定して生成する。
    /// @param degrees 角度（度）。
    /// @return 生成した角度。
    static constexpr Angle fromDegrees(float degrees) noexcept
    {
        return Angle{Degrees{degrees}};
    }

    /// 度角を指定して生成する。
    /// @param degrees 角度（度）。
    /// @return 生成した角度。
    static constexpr Angle fromDegrees(Degrees degrees) noexcept
    {
        return Angle{degrees};
    }

    /// 0 ラジアンを返す。
    /// @return ゼロ角度。
    static constexpr Angle zero() noexcept
    {
        return {};
    }

    /// ラジアン角を返す。
    /// @return 内部表現のラジアン角。
    constexpr Radians asRadians() const noexcept
    {
        return m_radians;
    }

    /// 度角を返す。
    /// @return 等価な度角。
    constexpr Degrees asDegrees() const noexcept
    {
        return m_radians.toDegrees();
    }

    /// 角度をラジアンで返す。
    /// @return 角度（ラジアン）。
    constexpr float radians() const noexcept
    {
        return m_radians.value;
    }

    /// 角度を度で返す。
    /// @return 角度（度）。
    constexpr float degrees() const noexcept
    {
        return asDegrees().value;
    }

    /// [0, 2π) の範囲に折り返した角度を返す。
    /// @return 等価な角度。
    Angle wrapped() const noexcept
    {
        constexpr double kTwoPiDouble = 2.0 * std::numbers::pi_v<double>;
        constexpr double kZeroTolerance = std::numeric_limits<float>::epsilon() * kTwoPiDouble;

        double result = std::fmod(static_cast<double>(m_radians.value), kTwoPiDouble);
        if (result < 0.0) {
            result += kTwoPiDouble;
        }
        if (result < kZeroTolerance) {
            result = 0.0;
        }
        return Angle{Radians{static_cast<float>(result)}};
    }

    /// 加算して自身を更新する。
    /// @param other 加える角度。
    /// @return 自身への参照。
    constexpr Angle& operator+=(const Angle& other) noexcept
    {
        m_radians.value += other.m_radians.value;
        return *this;
    }

    /// 減算して自身を更新する。
    /// @param other 引く角度。
    /// @return 自身への参照。
    constexpr Angle& operator-=(const Angle& other) noexcept
    {
        m_radians.value -= other.m_radians.value;
        return *this;
    }

    /// スカラー倍して自身を更新する。
    /// @param scalar 倍率。
    /// @return 自身への参照。
    constexpr Angle& operator*=(float scalar) noexcept
    {
        m_radians.value *= scalar;
        return *this;
    }

    /// スカラーで除算して自身を更新する。0 の場合はゼロ角度になる。
    /// @param scalar 除数。
    /// @return 自身への参照。
    constexpr Angle& operator/=(float scalar) noexcept
    {
        if (scalar == 0.0F) {
            m_radians.value = 0.0F;
            return *this;
        }
        m_radians.value /= scalar;
        return *this;
    }

    /// 全成分が完全一致するか判定する。
    /// @param other 比較対象。
    /// @return 一致すれば true。
    constexpr bool operator==(const Angle& other) const noexcept = default;

private:
    Radians m_radians{};
};

/// 2 つの角度を加算する。
/// @param lhs 左辺。
/// @param rhs 右辺。
/// @return 和の角度。
constexpr Angle operator+(const Angle& lhs, const Angle& rhs) noexcept
{
    return Angle::fromRadians(lhs.radians() + rhs.radians());
}

/// 2 つの角度を減算する。
/// @param lhs 左辺。
/// @param rhs 右辺。
/// @return 差の角度。
constexpr Angle operator-(const Angle& lhs, const Angle& rhs) noexcept
{
    return Angle::fromRadians(lhs.radians() - rhs.radians());
}

/// 角度の符号を反転する。
/// @param value 対象の角度。
/// @return 符号を反転した角度。
constexpr Angle operator-(const Angle& value) noexcept
{
    return Angle::fromRadians(-value.radians());
}

/// 角度をスカラー倍する。
/// @param value 対象の角度。
/// @param scalar 倍率。
/// @return scalar 倍した角度。
constexpr Angle operator*(const Angle& value, float scalar) noexcept
{
    return Angle::fromRadians(value.radians() * scalar);
}

/// スカラーと角度の積を返す。
/// @param scalar 倍率。
/// @param value 対象の角度。
/// @return scalar 倍した角度。
constexpr Angle operator*(float scalar, const Angle& value) noexcept
{
    return Angle::fromRadians(value.radians() * scalar);
}

/// 角度をスカラーで除算する。0 の場合はゼロ角度を返す。
/// @param value 対象の角度。
/// @param scalar 除数。
/// @return scalar で割った角度。
constexpr Angle operator/(const Angle& value, float scalar) noexcept
{
    if (scalar == 0.0F) {
        return Angle::zero();
    }
    return Angle::fromRadians(value.radians() / scalar);
}

}  // namespace sprout
