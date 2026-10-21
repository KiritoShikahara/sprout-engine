#pragma once

#include <chrono>

namespace sprout::detail {

/// @brief 高精度な時刻源の境界。
///
/// OS のタイマーを直接触る代わりに差し替え可能にしてある。
/// 返す値は単調増加し、精度は 1 マイクロ秒未満を想定する。
class TimeSource {
public:
    virtual ~TimeSource() = default;

    /// @brief 現在の経過時間を返す。
    /// @return 単調増加する経過時間。起点は OS の起動時刻。
    [[nodiscard]] virtual std::chrono::nanoseconds now() const = 0;
};

/// @brief OS の最高精度タイマーを使う時刻源を返す。
/// @return プロセス内で共有される時刻源。
TimeSource& systemTimeSource();

/// @brief 高精度クロック。
///
/// フレーム時間の計測などに使う。時刻源は差し替え可能なので、
/// テストでは決定的な値を返させられる。
///
/// @code
/// sprout::detail::Clock clock;
/// // ... 1 フレームの処理 ...
/// const double deltaSeconds = clock.elapsed();
/// clock.restart();
/// @endcode
class Clock {
public:
    /// OS の最高精度タイマーを使う。
    Clock() noexcept;

    /// 時刻源を差し替えて生成する（テスト用）。
    /// @param source 使用する時刻源。呼び出し側が寿命を管理する。
    explicit Clock(TimeSource& source) noexcept;

    /// @brief 現在の経過時間を返す。
    /// @return 単調増加する経過時間。起点は OS の起動時刻。
    [[nodiscard]] std::chrono::nanoseconds now() const;

    /// @brief 現在の経過時間を秒で返す。
    /// @return 経過秒数。
    [[nodiscard]] double seconds() const;

    /// @brief 前回 restart() 以降の経過時間を秒で返す。
    /// @return 経過秒数。restart() 直後は 0。
    [[nodiscard]] double elapsed() const;

    /// @brief 計測の開始点を現在時刻に戻す。
    void restart();

private:
    TimeSource* m_source;
    std::chrono::nanoseconds m_start;
};

}  // namespace sprout::detail
