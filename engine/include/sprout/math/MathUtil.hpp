#pragma once

#include <cmath>
#include <limits>

namespace sprout {

/// approxEqual() の既定許容誤差。
inline constexpr float kMathUtilDefaultTolerance = 1.0e-5F;

/// @brief 補間・比較・移動のための数値ユーティリティ。
///
/// スカラー値を対象とした小さな関数群を提供する。補間は範囲外の t を受け付けて
/// 外挿し、クランプが必要な場合は clamp() を併用する。
///
/// @code
/// const float value = sprout::lerp(0.0F, 100.0F, 0.25F); // 25
/// const float ratio = sprout::inverseLerp(0.0F, 100.0F, 25.0F); // 0.25
/// float velocity = 0.0F;
/// const float next = sprout::smoothDamp(0.0F, 10.0F, velocity, 0.3F, 1.0F / 60.0F);
/// @endcode

/// 2 つの値の間を線形補間する。
/// @param from t が 0 のときの値。
/// @param to t が 1 のときの値。
/// @param t 補間係数。範囲外の場合は外挿する。
/// @return from + (to - from) * t。
constexpr float lerp(float from, float to, float t) noexcept
{
    return from + (to - from) * t;
}

/// value が from と to の間で占める割合を返す。
/// @param from 割合 0 に対応する値。
/// @param to 割合 1 に対応する値。
/// @param value 割合を求める値。
/// @return (value - from) / (to - from)。from と to が等しい場合は 0。
constexpr float inverseLerp(float from, float to, float value) noexcept
{
    if (from == to) {
        return 0.0F;
    }
    return (value - from) / (to - from);
}

/// value を最小値と最大値の範囲に収める。
/// @param value 対象の値。
/// @param minimum 下限。maximum 以下であること。
/// @param maximum 上限。
/// @return 範囲内に収めた値。
constexpr float clamp(float value, float minimum, float maximum) noexcept
{
    if (value < minimum) {
        return minimum;
    }
    if (value > maximum) {
        return maximum;
    }
    return value;
}

/// 許容誤差つきで 2 つの値が等しいか判定する。
/// @param lhs 左辺。
/// @param rhs 右辺。
/// @param tolerance 許容誤差。既定は kMathUtilDefaultTolerance。
/// @return 差の絶対値が tolerance 以下なら true。
inline bool approxEqual(float lhs, float rhs, float tolerance = kMathUtilDefaultTolerance) noexcept
{
    return std::abs(lhs - rhs) <= tolerance;
}

/// 目標値へ滑らかに近づけた値を返す（Unity の SmoothDamp と同じ減衰特性）。
/// @param current 現在の値。
/// @param target 目標値。
/// @param currentVelocity 現在の速度。更新される。
/// @param smoothTime 目標値へおおよそ到達するまでの時間（秒）。0 以下なら即座に到達する。
/// @param maxSpeed 変化量の上限。
/// @param deltaTime 経過時間（秒）。
/// @return 次の値。
inline float smoothDamp(float current,
                        float target,
                        float& currentVelocity,
                        float smoothTime,
                        float maxSpeed,
                        float deltaTime) noexcept
{
    if (smoothTime <= 0.0F) {
        currentVelocity = 0.0F;
        return target;
    }
    if (deltaTime <= 0.0F) {
        return current;
    }

    const float omega = 2.0F / smoothTime;
    const float x = omega * deltaTime;
    const float damping = 1.0F / (1.0F + x + 0.48F * x * x + 0.235F * x * x * x);

    float change = current - target;
    const float originalTarget = target;
    const float limitedSpeed = maxSpeed < 0.0F ? 0.0F : maxSpeed;
    const float maxChange = limitedSpeed * smoothTime;
    change = clamp(change, -maxChange, maxChange);
    target = current - change;

    const float temp = (currentVelocity + omega * change) * deltaTime;
    currentVelocity = (currentVelocity - omega * temp) * damping;
    float output = target + (change + temp) * damping;

    if (((originalTarget - current) > 0.0F) == (output > originalTarget)) {
        output = originalTarget;
        currentVelocity = (output - originalTarget) / deltaTime;
    }
    return output;
}

/// 目標値へ滑らかに近づけた値を返す。速度上限を設けない簡易版。
/// @param current 現在の値。
/// @param target 目標値。
/// @param currentVelocity 現在の速度。更新される。
/// @param smoothTime 目標値へおおよそ到達するまでの時間（秒）。0 以下なら即座に到達する。
/// @param deltaTime 経過時間（秒）。
/// @return 次の値。
inline float smoothDamp(float current, float target, float& currentVelocity, float smoothTime, float deltaTime) noexcept
{
    return smoothDamp(current, target, currentVelocity, smoothTime, std::numeric_limits<float>::infinity(), deltaTime);
}

/// 現在の値から目標値へ最大 maxDelta だけ近づけた値を返す。
/// @param current 現在の値。
/// @param target 目標値。
/// @param maxDelta 1 回の移動量の上限。負の場合は移動しない。
/// @return 移動後の値。maxDelta 以内で到達できる場合は target。
constexpr float moveTowards(float current, float target, float maxDelta) noexcept
{
    if (maxDelta <= 0.0F) {
        return current;
    }
    const float difference = target - current;
    if (difference > maxDelta) {
        return current + maxDelta;
    }
    if (difference < -maxDelta) {
        return current - maxDelta;
    }
    return target;
}

}  // namespace sprout
