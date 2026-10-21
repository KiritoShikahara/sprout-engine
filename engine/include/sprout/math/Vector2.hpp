#pragma once

#include <cmath>

namespace sprout {

/// @brief 2D ベクトル（単精度浮動小数点）。
///
/// 位置・速度・方向などを表す基本型。全ての演算は値型で、例外を投げない。
///
/// @code
/// const sprout::Vector2 velocity{3.0F, 4.0F};
/// const float speed = velocity.length();                   // 5.0
/// const sprout::Vector2 direction = velocity.normalized(); // (0.6, 0.8)
/// const float forward = velocity.dot(sprout::Vector2::right());
/// @endcode
struct Vector2 {
    /// X 成分。
    float x{0.0F};

    /// Y 成分。
    float y{0.0F};

    /// 正規化時にゼロとみなす長さ。
    static constexpr float kMinimumLength = 1.0e-6F;

    /// isApproxEqual() の既定許容誤差。
    static constexpr float kDefaultTolerance = 1.0e-5F;

    /// (0, 0) を生成する。
    constexpr Vector2() noexcept = default;

    /// 成分を指定して生成する。
    /// @param xValue X 成分。
    /// @param yValue Y 成分。
    constexpr Vector2(float xValue, float yValue) noexcept
        : x{xValue}
        , y{yValue}
    {
    }

    /// (0, 0) を返す。
    /// @return ゼロベクトル。
    static constexpr Vector2 zero() noexcept
    {
        return {};
    }

    /// (1, 1) を返す。
    /// @return 全成分が 1 のベクトル。
    static constexpr Vector2 one() noexcept
    {
        return {1.0F, 1.0F};
    }

    /// X 軸の正方向 (1, 0) を返す。
    /// @return 右方向の単位ベクトル。
    static constexpr Vector2 right() noexcept
    {
        return {1.0F, 0.0F};
    }

    /// Y 軸の正方向 (0, 1) を返す。
    /// @return 上方向の単位ベクトル。
    static constexpr Vector2 up() noexcept
    {
        return {0.0F, 1.0F};
    }

    /// 長さの 2 乗を返す（平方根を避けて比較したい場合に使う）。
    /// @return x^2 + y^2。
    constexpr float lengthSquared() const noexcept
    {
        return x * x + y * y;
    }

    /// 長さ（ユークリッドノルム）を返す。
    /// @return ベクトルの長さ。
    float length() const noexcept
    {
        return std::sqrt(lengthSquared());
    }

    /// 長さ 1 のベクトルを返す。
    /// @return 正規化したベクトル。ゼロベクトルの場合は (0, 0)。
    Vector2 normalized() const noexcept
    {
        const float currentLength = length();
        if (currentLength <= kMinimumLength) {
            return zero();
        }
        return {x / currentLength, y / currentLength};
    }

    /// 内積を返す。
    /// @param other 相手のベクトル。
    /// @return x * other.x + y * other.y。
    constexpr float dot(const Vector2& other) const noexcept
    {
        return x * other.x + y * other.y;
    }

    /// 2D 外積（3D 外積の z 成分）を返す。
    /// @param other 相手のベクトル。
    /// @return x * other.y - y * other.x。
    constexpr float cross(const Vector2& other) const noexcept
    {
        return x * other.y - y * other.x;
    }

    /// 反時計回りに回転したベクトルを返す。
    /// @param radians 回転角（ラジアン）。
    /// @return 回転後のベクトル。
    Vector2 rotated(float radians) const noexcept
    {
        const float cosine = std::cos(radians);
        const float sine = std::sin(radians);
        return {x * cosine - y * sine, x * sine + y * cosine};
    }

    /// 正の X 軸からの角度を返す。
    /// @return 角度（ラジアン、-π 以上 π 以下）。
    float angle() const noexcept
    {
        return std::atan2(y, x);
    }

    /// 許容誤差つきで等しいか判定する。
    /// @param other 比較対象。
    /// @param tolerance 許容誤差。既定は kDefaultTolerance。
    /// @return 全成分の差が tolerance 以下なら true。
    bool isApproxEqual(const Vector2& other, float tolerance = kDefaultTolerance) const noexcept
    {
        return std::abs(x - other.x) <= tolerance && std::abs(y - other.y) <= tolerance;
    }

    /// 加算して自身を更新する。
    /// @param other 加えるベクトル。
    /// @return 自身への参照。
    constexpr Vector2& operator+=(const Vector2& other) noexcept
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    /// 減算して自身を更新する。
    /// @param other 引くベクトル。
    /// @return 自身への参照。
    constexpr Vector2& operator-=(const Vector2& other) noexcept
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    /// スカラー倍して自身を更新する。
    /// @param scalar 倍率。
    /// @return 自身への参照。
    constexpr Vector2& operator*=(float scalar) noexcept
    {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    /// スカラーで除算して自身を更新する。0 の場合はゼロベクトルになる。
    /// @param scalar 除数。
    /// @return 自身への参照。
    constexpr Vector2& operator/=(float scalar) noexcept
    {
        if (scalar == 0.0F) {
            x = 0.0F;
            y = 0.0F;
            return *this;
        }
        x /= scalar;
        y /= scalar;
        return *this;
    }

    /// 全成分が完全一致するか判定する。
    /// @param other 比較対象。
    /// @return 一致すれば true。
    constexpr bool operator==(const Vector2& other) const noexcept = default;
};

/// 2 つのベクトルを加算する。
/// @param lhs 左辺。
/// @param rhs 右辺。
/// @return 成分ごとの和。
constexpr Vector2 operator+(const Vector2& lhs, const Vector2& rhs) noexcept
{
    return {lhs.x + rhs.x, lhs.y + rhs.y};
}

/// 2 つのベクトルを減算する。
/// @param lhs 左辺。
/// @param rhs 右辺。
/// @return 成分ごとの差。
constexpr Vector2 operator-(const Vector2& lhs, const Vector2& rhs) noexcept
{
    return {lhs.x - rhs.x, lhs.y - rhs.y};
}

/// ベクトルの符号を反転する。
/// @param value 対象のベクトル。
/// @return 全成分を反転したベクトル。
constexpr Vector2 operator-(const Vector2& value) noexcept
{
    return {-value.x, -value.y};
}

/// ベクトルをスカラー倍する。
/// @param value 対象のベクトル。
/// @param scalar 倍率。
/// @return 各成分を scalar 倍したベクトル。
constexpr Vector2 operator*(const Vector2& value, float scalar) noexcept
{
    return {value.x * scalar, value.y * scalar};
}

/// スカラーとベクトルの積を返す。
/// @param scalar 倍率。
/// @param value 対象のベクトル。
/// @return 各成分を scalar 倍したベクトル。
constexpr Vector2 operator*(float scalar, const Vector2& value) noexcept
{
    return {value.x * scalar, value.y * scalar};
}

/// ベクトルをスカラーで除算する。0 の場合はゼロベクトルを返す。
/// @param value 対象のベクトル。
/// @param scalar 除数。
/// @return 各成分を scalar で割ったベクトル。
constexpr Vector2 operator/(const Vector2& value, float scalar) noexcept
{
    if (scalar == 0.0F) {
        return Vector2::zero();
    }
    return {value.x / scalar, value.y / scalar};
}

/// @brief 整数 2D ベクトル。
///
/// タイル座標・グリッド位置など、整数の座標を表す。
///
/// @code
/// const sprout::Vector2Int tile{3, 4};
/// const int distanceSquared = tile.lengthSquared(); // 25
/// const sprout::Vector2Int moved = tile + sprout::Vector2Int{1, 0};
/// @endcode
struct Vector2Int {
    /// X 成分。
    int x{0};

    /// Y 成分。
    int y{0};

    /// (0, 0) を生成する。
    constexpr Vector2Int() noexcept = default;

    /// 成分を指定して生成する。
    /// @param xValue X 成分。
    /// @param yValue Y 成分。
    constexpr Vector2Int(int xValue, int yValue) noexcept
        : x{xValue}
        , y{yValue}
    {
    }

    /// (0, 0) を返す。
    /// @return ゼロベクトル。
    static constexpr Vector2Int zero() noexcept
    {
        return {};
    }

    /// (1, 1) を返す。
    /// @return 全成分が 1 のベクトル。
    static constexpr Vector2Int one() noexcept
    {
        return {1, 1};
    }

    /// 長さの 2 乗を返す。
    /// @return x^2 + y^2。
    constexpr int lengthSquared() const noexcept
    {
        return x * x + y * y;
    }

    /// 長さ（ユークリッドノルム）を返す。
    /// @return ベクトルの長さ。
    float length() const noexcept
    {
        return std::sqrt(static_cast<float>(lengthSquared()));
    }

    /// 内積を返す。
    /// @param other 相手のベクトル。
    /// @return x * other.x + y * other.y。
    constexpr int dot(const Vector2Int& other) const noexcept
    {
        return x * other.x + y * other.y;
    }

    /// 2D 外積（3D 外積の z 成分）を返す。
    /// @param other 相手のベクトル。
    /// @return x * other.y - y * other.x。
    constexpr int cross(const Vector2Int& other) const noexcept
    {
        return x * other.y - y * other.x;
    }

    /// ゼロベクトルか判定する。
    /// @return (0, 0) なら true。
    constexpr bool isZero() const noexcept
    {
        return x == 0 && y == 0;
    }

    /// 加算して自身を更新する。
    /// @param other 加えるベクトル。
    /// @return 自身への参照。
    constexpr Vector2Int& operator+=(const Vector2Int& other) noexcept
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    /// 減算して自身を更新する。
    /// @param other 引くベクトル。
    /// @return 自身への参照。
    constexpr Vector2Int& operator-=(const Vector2Int& other) noexcept
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    /// スカラー倍して自身を更新する。
    /// @param scalar 倍率。
    /// @return 自身への参照。
    constexpr Vector2Int& operator*=(int scalar) noexcept
    {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    /// スカラーで除算して自身を更新する。0 の場合はゼロベクトルになる。
    /// @param scalar 除数。
    /// @return 自身への参照。
    constexpr Vector2Int& operator/=(int scalar) noexcept
    {
        if (scalar == 0) {
            x = 0;
            y = 0;
            return *this;
        }
        x /= scalar;
        y /= scalar;
        return *this;
    }

    /// 全成分が完全一致するか判定する。
    /// @param other 比較対象。
    /// @return 一致すれば true。
    constexpr bool operator==(const Vector2Int& other) const noexcept = default;
};

/// 2 つの整数ベクトルを加算する。
/// @param lhs 左辺。
/// @param rhs 右辺。
/// @return 成分ごとの和。
constexpr Vector2Int operator+(const Vector2Int& lhs, const Vector2Int& rhs) noexcept
{
    return {lhs.x + rhs.x, lhs.y + rhs.y};
}

/// 2 つの整数ベクトルを減算する。
/// @param lhs 左辺。
/// @param rhs 右辺。
/// @return 成分ごとの差。
constexpr Vector2Int operator-(const Vector2Int& lhs, const Vector2Int& rhs) noexcept
{
    return {lhs.x - rhs.x, lhs.y - rhs.y};
}

/// 整数ベクトルの符号を反転する。
/// @param value 対象のベクトル。
/// @return 全成分を反転したベクトル。
constexpr Vector2Int operator-(const Vector2Int& value) noexcept
{
    return {-value.x, -value.y};
}

/// 整数ベクトルをスカラー倍する。
/// @param value 対象のベクトル。
/// @param scalar 倍率。
/// @return 各成分を scalar 倍したベクトル。
constexpr Vector2Int operator*(const Vector2Int& value, int scalar) noexcept
{
    return {value.x * scalar, value.y * scalar};
}

/// スカラーと整数ベクトルの積を返す。
/// @param scalar 倍率。
/// @param value 対象のベクトル。
/// @return 各成分を scalar 倍したベクトル。
constexpr Vector2Int operator*(int scalar, const Vector2Int& value) noexcept
{
    return {value.x * scalar, value.y * scalar};
}

/// 整数ベクトルをスカラーで除算する。0 の場合はゼロベクトルを返す。
/// @param value 対象のベクトル。
/// @param scalar 除数。
/// @return 各成分を scalar で割ったベクトル。
constexpr Vector2Int operator/(const Vector2Int& value, int scalar) noexcept
{
    if (scalar == 0) {
        return Vector2Int::zero();
    }
    return {value.x / scalar, value.y / scalar};
}

}  // namespace sprout
