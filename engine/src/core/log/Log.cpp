#include "sprout/core/Log.hpp"

#include <algorithm>
#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace sprout {
namespace {

struct LogState {
    std::mutex mutex;
    std::vector<std::shared_ptr<LogSink>> sinks;
    std::unordered_map<std::string, bool> categoryFilters;
};

LogState& state()
{
    static LogState instance;
    return instance;
}

std::atomic<int> g_minimumLevel{static_cast<int>(LogLevel::Trace)};

}  // namespace

std::string_view toString(LogLevel level) noexcept
{
    switch (level) {
    case LogLevel::Trace:
        return "TRACE";
    case LogLevel::Info:
        return "INFO";
    case LogLevel::Warning:
        return "WARNING";
    case LogLevel::Error:
        return "ERROR";
    case LogLevel::Fatal:
        return "FATAL";
    }
    return "UNKNOWN";
}

std::string formatLogRecord(const LogRecord& record)
{
    return std::format("[{}] [{}] {}", toString(record.level), record.category, record.message);
}

void Log::addSink(std::shared_ptr<LogSink> sink)
{
    if (!sink) {
        return;
    }
    LogState& logState = state();
    const std::lock_guard lock(logState.mutex);
    logState.sinks.push_back(std::move(sink));
}

void Log::removeSink(const std::shared_ptr<LogSink>& sink)
{
    LogState& logState = state();
    const std::lock_guard lock(logState.mutex);
    logState.sinks.erase(std::remove(logState.sinks.begin(), logState.sinks.end(), sink), logState.sinks.end());
}

void Log::clearSinks()
{
    LogState& logState = state();
    const std::lock_guard lock(logState.mutex);
    logState.sinks.clear();
}

void Log::setMinimumLevel(LogLevel level) noexcept
{
    g_minimumLevel.store(static_cast<int>(level), std::memory_order_relaxed);
}

LogLevel Log::minimumLevel() noexcept
{
    return static_cast<LogLevel>(g_minimumLevel.load(std::memory_order_relaxed));
}

void Log::setCategoryEnabled(std::string_view category, bool enabled)
{
    LogState& logState = state();
    const std::lock_guard lock(logState.mutex);
    logState.categoryFilters[std::string(category)] = enabled;
}

bool Log::isCategoryEnabled(std::string_view category)
{
    LogState& logState = state();
    const std::lock_guard lock(logState.mutex);
    const auto it = logState.categoryFilters.find(std::string(category));
    return it == logState.categoryFilters.end() || it->second;
}

void Log::clearCategoryFilters()
{
    LogState& logState = state();
    const std::lock_guard lock(logState.mutex);
    logState.categoryFilters.clear();
}

bool Log::isEnabled(LogLevel level, std::string_view category)
{
    if (static_cast<int>(level) < g_minimumLevel.load(std::memory_order_relaxed)) {
        return false;
    }
    return isCategoryEnabled(category);
}

void Log::emit(const LogRecord& record)
{
    std::vector<std::shared_ptr<LogSink>> sinks;
    {
        LogState& logState = state();
        const std::lock_guard lock(logState.mutex);
        sinks = logState.sinks;
    }

    for (const std::shared_ptr<LogSink>& sink : sinks) {
        try {
            sink->write(record);
        } catch (...) {
        }
    }
}

}  // namespace sprout
