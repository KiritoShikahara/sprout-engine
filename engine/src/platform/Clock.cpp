#include "platform/Clock.hpp"

#include <chrono>

namespace {

/// @brief OS の最高精度タイマーを使う時刻源。
class SystemTimeSource final : public sprout::detail::TimeSource {
public:
    [[nodiscard]] std::chrono::nanoseconds now() const override
    {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now().time_since_epoch());
    }
};

}  // namespace

namespace sprout::detail {

TimeSource& systemTimeSource()
{
    static SystemTimeSource source;
    return source;
}

Clock::Clock() noexcept
    : m_source{&systemTimeSource()}
    , m_start{m_source->now()}
{
}

Clock::Clock(TimeSource& source) noexcept
    : m_source{&source}
    , m_start{source.now()}
{
}

std::chrono::nanoseconds Clock::now() const
{
    return m_source->now();
}

double Clock::seconds() const
{
    return std::chrono::duration<double>(now()).count();
}

double Clock::elapsed() const
{
    return std::chrono::duration<double>(now() - m_start).count();
}

void Clock::restart()
{
    m_start = now();
}

}  // namespace sprout::detail
