#include "platform/Clock.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <thread>

namespace {

using sprout::detail::Clock;
using sprout::detail::TimeSource;

/// @brief 時刻を手で進められるテスト用の時刻源。
class FakeTimeSource final : public TimeSource {
public:
    [[nodiscard]] std::chrono::nanoseconds now() const override
    {
        return m_now;
    }

    void advance(std::chrono::nanoseconds amount)
    {
        m_now += amount;
    }

    void set(std::chrono::nanoseconds value)
    {
        m_now = value;
    }

private:
    std::chrono::nanoseconds m_now{std::chrono::nanoseconds{0}};
};

TEST(ClockTests, Now_UsesInjectedSource)
{
    FakeTimeSource source;
    Clock clock{source};
    source.set(std::chrono::nanoseconds{1'500'000'000});

    EXPECT_EQ(clock.now().count(), 1'500'000'000);
    EXPECT_DOUBLE_EQ(clock.seconds(), 1.5);
}

TEST(ClockTests, Elapsed_IsZeroAtStart)
{
    FakeTimeSource source;
    const Clock clock{source};

    EXPECT_DOUBLE_EQ(clock.elapsed(), 0.0);
}

TEST(ClockTests, Elapsed_ReflectsSourceAdvance)
{
    FakeTimeSource source;
    Clock clock{source};

    source.advance(std::chrono::milliseconds{250});

    EXPECT_DOUBLE_EQ(clock.elapsed(), 0.25);
}

TEST(ClockTests, Restart_ResetsElapsed)
{
    FakeTimeSource source;
    Clock clock{source};
    source.advance(std::chrono::milliseconds{100});

    clock.restart();

    EXPECT_DOUBLE_EQ(clock.elapsed(), 0.0);

    source.advance(std::chrono::milliseconds{50});

    EXPECT_DOUBLE_EQ(clock.elapsed(), 0.05);
}

TEST(ClockTests, Seconds_KeepsSubMillisecondPrecision)
{
    FakeTimeSource source;
    Clock clock{source};
    source.set(std::chrono::nanoseconds{1'234'567});

    EXPECT_DOUBLE_EQ(clock.seconds(), 0.001234567);
}

TEST(ClockTests, SystemTimeSource_IsShared)
{
    EXPECT_EQ(&sprout::detail::systemTimeSource(), &sprout::detail::systemTimeSource());
}

TEST(ClockTests, SystemClock_AdvancesWithRealTime)
{
    Clock clock;
    const auto before = clock.now();

    std::this_thread::sleep_for(std::chrono::milliseconds{20});

    EXPECT_GT(clock.now().count(), before.count());
    EXPECT_GE(clock.elapsed(), 0.02);
}

TEST(ClockTests, SystemClock_IsMonotonic)
{
    Clock clock;
    auto previous = clock.now();

    for (int i = 0; i < 1000; ++i) {
        const auto current = clock.now();
        EXPECT_GE(current.count(), previous.count());
        previous = current;
    }
}

TEST(ClockTests, SystemClock_HasSubMillisecondResolution)
{
    Clock clock;
    auto previous = clock.now();

    for (int i = 0; i < 10000; ++i) {
        const auto current = clock.now();
        const auto delta = (current - previous).count();
        previous = current;
        if (delta > 0) {
            EXPECT_LT(delta, 1'000'000);
            return;
        }
    }

    FAIL() << "1 ミリ秒未満の分解能を持つ時刻が見つからなかった";
}

}  // namespace
