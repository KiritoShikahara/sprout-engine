#pragma once

namespace sprout {

/// @brief 代表的なイージング関数（30 種）。
///
/// 入力 t は 0〜1 にクランプされる。全ての関数は t = 0 で 0、t = 1 で 1 を返す。
/// Back / Elastic / Bounce は途中で 0〜1 の範囲を超えることがある。
///
/// @code
/// const float half = sprout::easeOutQuad(0.5F); // 0.75
/// @endcode

/// 正弦波の加速（開始が緩やか）。
/// @param t 進行度。0〜1 の範囲外はクランプされる。
/// @return イージング後の値。
float easeInSine(float t) noexcept;

/// 正弦波の減速（終了が緩やか）。
/// @param t 進行度。0〜1 の範囲外はクランプされる。
/// @return イージング後の値。
float easeOutSine(float t) noexcept;

/// 正弦波の加減速（開始と終了が緩やか）。
/// @param t 進行度。0〜1 の範囲外はクランプされる。
/// @return イージング後の値。
float easeInOutSine(float t) noexcept;

/// 2 次の加速。
/// @param t 進行度。0〜1 の範囲外はクランプされる。
/// @return イージング後の値。
float easeInQuad(float t) noexcept;

/// 2 次の減速。
/// @param t 進行度。0〜1 の範囲外はクランプされる。
/// @return イージング後の値。
float easeOutQuad(float t) noexcept;

/// 2 次の加減速。
/// @param t 進行度。0〜1 の範囲外はクランプされる。
/// @return イージング後の値。
float easeInOutQuad(float t) noexcept;

/// 3 次の加速。
/// @param t 進行度。0〜1 の範囲外はクランプされる。
/// @return イージング後の値。
float easeInCubic(float t) noexcept;

/// 3 次の減速。
/// @param t 進行度。0〜1 の範囲外はクランプされる。
/// @return イージング後の値。
float easeOutCubic(float t) noexcept;

/// 3 次の加減速。
/// @param t 進行度。0〜1 の範囲外はクランプされる。
/// @return イージング後の値。
float easeInOutCubic(float t) noexcept;

/// 4 次の加速。
/// @param t 進行度。0〜1 の範囲外はクランプされる。
/// @return イージング後の値。
float easeInQuart(float t) noexcept;

/// 4 次の減速。
/// @param t 進行度。0〜1 の範囲外はクランプされる。
/// @return イージング後の値。
float easeOutQuart(float t) noexcept;

/// 4 次の加減速。
/// @param t 進行度。0〜1 の範囲外はクランプされる。
/// @return イージング後の値。
float easeInOutQuart(float t) noexcept;

/// 5 次の加速。
/// @param t 進行度。0〜1 の範囲外はクランプされる。
/// @return イージング後の値。
float easeInQuint(float t) noexcept;

/// 5 次の減速。
/// @param t 進行度。0〜1 の範囲外はクランプされる。
/// @return イージング後の値。
float easeOutQuint(float t) noexcept;

/// 5 次の加減速。
/// @param t 進行度。0〜1 の範囲外はクランプされる。
/// @return イージング後の値。
float easeInOutQuint(float t) noexcept;

/// 指数関数の加速。
/// @param t 進行度。0〜1 の範囲外はクランプされる。
/// @return イージング後の値。
float easeInExpo(float t) noexcept;

/// 指数関数の減速。
/// @param t 進行度。0〜1 の範囲外はクランプされる。
/// @return イージング後の値。
float easeOutExpo(float t) noexcept;

/// 指数関数の加減速。
/// @param t 進行度。0〜1 の範囲外はクランプされる。
/// @return イージング後の値。
float easeInOutExpo(float t) noexcept;

/// 円弧の加速。
/// @param t 進行度。0〜1 の範囲外はクランプされる。
/// @return イージング後の値。
float easeInCirc(float t) noexcept;

/// 円弧の減速。
/// @param t 進行度。0〜1 の範囲外はクランプされる。
/// @return イージング後の値。
float easeOutCirc(float t) noexcept;

/// 円弧の加減速。
/// @param t 進行度。0〜1 の範囲外はクランプされる。
/// @return イージング後の値。
float easeInOutCirc(float t) noexcept;

/// 開始時に一度戻る加速（オーバーシュート）。
/// @param t 進行度。0〜1 の範囲外はクランプされる。
/// @return イージング後の値。途中で負になることがある。
float easeInBack(float t) noexcept;

/// 終了時に一度行き過ぎる減速（オーバーシュート）。
/// @param t 進行度。0〜1 の範囲外はクランプされる。
/// @return イージング後の値。途中で 1 を超えることがある。
float easeOutBack(float t) noexcept;

/// 両端で行き過ぎる加減速（オーバーシュート）。
/// @param t 進行度。0〜1 の範囲外はクランプされる。
/// @return イージング後の値。途中で 0〜1 の範囲を超えることがある。
float easeInOutBack(float t) noexcept;

/// ばねのような加速。
/// @param t 進行度。0〜1 の範囲外はクランプされる。
/// @return イージング後の値。途中で負になることがある。
float easeInElastic(float t) noexcept;

/// ばねのような減速。
/// @param t 進行度。0〜1 の範囲外はクランプされる。
/// @return イージング後の値。途中で 1 を超えることがある。
float easeOutElastic(float t) noexcept;

/// ばねのような加減速。
/// @param t 進行度。0〜1 の範囲外はクランプされる。
/// @return イージング後の値。途中で 0〜1 の範囲を超えることがある。
float easeInOutElastic(float t) noexcept;

/// 跳ねるような加速。
/// @param t 進行度。0〜1 の範囲外はクランプされる。
/// @return イージング後の値。
float easeInBounce(float t) noexcept;

/// 跳ねるような減速。
/// @param t 進行度。0〜1 の範囲外はクランプされる。
/// @return イージング後の値。
float easeOutBounce(float t) noexcept;

/// 跳ねるような加減速。
/// @param t 進行度。0〜1 の範囲外はクランプされる。
/// @return イージング後の値。
float easeInOutBounce(float t) noexcept;

}  // namespace sprout
