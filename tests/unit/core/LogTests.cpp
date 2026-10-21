#include "sprout/core/Log.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <format>
#include <fstream>
#include <iterator>
#include <memory>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

namespace {

using sprout::CallbackSink;
using sprout::ConsoleSink;
using sprout::FileSink;
using sprout::Log;
using sprout::LogLevel;
using sprout::LogRecord;

class RecordCollector {
public:
    void append(const LogRecord& record)
    {
        const std::lock_guard lock(m_mutex);
        m_records.push_back(record);
    }

    [[nodiscard]] std::vector<LogRecord> records() const
    {
        const std::lock_guard lock(m_mutex);
        return m_records;
    }

private:
    mutable std::mutex m_mutex;
    std::vector<LogRecord> m_records;
};

std::shared_ptr<CallbackSink> makeSink(RecordCollector& collector)
{
    return std::make_shared<CallbackSink>(
        [&collector](const LogRecord& record) { collector.append(record); });
}

std::filesystem::path makeTempPath(std::string_view name)
{
    static std::atomic<unsigned> counter{0};
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path()
           / ("sprout_log_" + std::string(name) + "_" + std::to_string(stamp) + "_"
              + std::to_string(counter.fetch_add(1, std::memory_order_relaxed)) + ".log");
}

std::string readFile(const std::filesystem::path& path)
{
    std::ifstream input(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}

class LogTests : public ::testing::Test {
protected:
    void SetUp() override
    {
        Log::clearSinks();
        Log::setMinimumLevel(LogLevel::Trace);
        Log::clearCategoryFilters();
    }

    void TearDown() override
    {
        Log::clearSinks();
        Log::setMinimumLevel(LogLevel::Trace);
        Log::clearCategoryFilters();
    }
};

}  // namespace

TEST_F(LogTests, ToString_EachLevel_ReturnsUppercaseName)
{
    EXPECT_EQ(sprout::toString(LogLevel::Trace), std::string_view{"TRACE"});
    EXPECT_EQ(sprout::toString(LogLevel::Info), std::string_view{"INFO"});
    EXPECT_EQ(sprout::toString(LogLevel::Warning), std::string_view{"WARNING"});
    EXPECT_EQ(sprout::toString(LogLevel::Error), std::string_view{"ERROR"});
    EXPECT_EQ(sprout::toString(LogLevel::Fatal), std::string_view{"FATAL"});
}

TEST_F(LogTests, Info_FormattedArguments_ProducesFormattedMessage)
{
    RecordCollector collector;
    Log::addSink(makeSink(collector));

    Log::info("player={} score={:.2f}", std::string_view{"Mika"}, 12.5);

    const std::vector<LogRecord> records = collector.records();
    ASSERT_EQ(records.size(), 1U);
    EXPECT_EQ(records.front().level, LogLevel::Info);
    EXPECT_EQ(records.front().category, "General");
    EXPECT_EQ(records.front().message, "player=Mika score=12.50");
}

TEST_F(LogTests, ConvenienceMethods_EachLevel_EmitsMatchingLevel)
{
    RecordCollector collector;
    Log::addSink(makeSink(collector));

    Log::trace("t");
    Log::info("i");
    Log::warning("w");
    Log::error("e");
    Log::fatal("f");

    const std::vector<LogRecord> records = collector.records();
    ASSERT_EQ(records.size(), 5U);
    EXPECT_EQ(records[0].level, LogLevel::Trace);
    EXPECT_EQ(records[0].message, "t");
    EXPECT_EQ(records[1].level, LogLevel::Info);
    EXPECT_EQ(records[1].message, "i");
    EXPECT_EQ(records[2].level, LogLevel::Warning);
    EXPECT_EQ(records[2].message, "w");
    EXPECT_EQ(records[3].level, LogLevel::Error);
    EXPECT_EQ(records[3].message, "e");
    EXPECT_EQ(records[4].level, LogLevel::Fatal);
    EXPECT_EQ(records[4].message, "f");
}

TEST_F(LogTests, MinimumLevel_Warning_SuppressesLowerLevels)
{
    RecordCollector collector;
    Log::addSink(makeSink(collector));
    Log::setMinimumLevel(LogLevel::Warning);

    EXPECT_EQ(Log::minimumLevel(), LogLevel::Warning);
    Log::trace("t");
    Log::info("i");
    Log::warning("w");
    Log::error("e");
    Log::fatal("f");

    const std::vector<LogRecord> records = collector.records();
    ASSERT_EQ(records.size(), 3U);
    EXPECT_EQ(records[0].level, LogLevel::Warning);
    EXPECT_EQ(records[1].level, LogLevel::Error);
    EXPECT_EQ(records[2].level, LogLevel::Fatal);
}

TEST_F(LogTests, CategoryFilter_Disabled_SuppressesOnlyThatCategory)
{
    RecordCollector collector;
    Log::addSink(makeSink(collector));

    Log::setCategoryEnabled("Physics", false);
    EXPECT_FALSE(Log::isCategoryEnabled("Physics"));
    EXPECT_TRUE(Log::isCategoryEnabled("Graphics"));

    Log::log(LogLevel::Info, "Physics", "hidden {}", 1);
    Log::log(LogLevel::Info, "Graphics", "shown {}", 2);

    ASSERT_EQ(collector.records().size(), 1U);
    EXPECT_EQ(collector.records().front().message, "shown 2");
    EXPECT_EQ(collector.records().front().category, "Graphics");

    Log::setCategoryEnabled("Physics", true);
    Log::log(LogLevel::Info, "Physics", "visible {}", 3);

    ASSERT_EQ(collector.records().size(), 2U);
    EXPECT_EQ(collector.records().back().message, "visible 3");
    EXPECT_EQ(collector.records().back().category, "Physics");
}

TEST_F(LogTests, MultipleSinks_EachReceivesSameRecord)
{
    RecordCollector first;
    RecordCollector second;
    Log::addSink(makeSink(first));
    Log::addSink(makeSink(second));

    Log::info("shared {}", 3);

    const std::vector<LogRecord> firstRecords = first.records();
    const std::vector<LogRecord> secondRecords = second.records();
    ASSERT_EQ(firstRecords.size(), 1U);
    ASSERT_EQ(secondRecords.size(), 1U);
    EXPECT_EQ(firstRecords.front().message, "shared 3");
    EXPECT_EQ(secondRecords.front().message, "shared 3");
    EXPECT_EQ(secondRecords.front().level, LogLevel::Info);
}

TEST_F(LogTests, RemoveSink_RemovedSinkReceivesNothing)
{
    RecordCollector first;
    RecordCollector second;
    const std::shared_ptr<CallbackSink> firstSink = makeSink(first);
    Log::addSink(firstSink);
    Log::addSink(makeSink(second));

    Log::removeSink(firstSink);
    Log::info("after removal");

    EXPECT_TRUE(first.records().empty());
    ASSERT_EQ(second.records().size(), 1U);
    EXPECT_EQ(second.records().front().message, "after removal");
}

TEST_F(LogTests, ClearSinks_NoSinkReceivesMessages)
{
    RecordCollector collector;
    Log::addSink(makeSink(collector));

    Log::clearSinks();
    Log::info("discarded");

    EXPECT_TRUE(collector.records().empty());
}

TEST_F(LogTests, ConsoleSink_WritesLevelCategoryAndMessage)
{
    std::ostringstream stream;
    Log::addSink(std::make_shared<ConsoleSink>(stream));

    Log::log(LogLevel::Warning, "Storage", "disk {}%", 90);

    const std::string output = stream.str();
    ASSERT_FALSE(output.empty());
    EXPECT_NE(output.find("[WARNING]"), std::string::npos);
    EXPECT_NE(output.find("[Storage]"), std::string::npos);
    EXPECT_NE(output.find("disk 90%"), std::string::npos);
    EXPECT_EQ(output.back(), '\n');
}

TEST_F(LogTests, FileSink_WritesFormattedRecord)
{
    const std::filesystem::path path = makeTempPath("formatted");
    {
        const std::shared_ptr<FileSink> sink = std::make_shared<FileSink>(path);
        ASSERT_TRUE(sink->isOpen());
        Log::addSink(sink);
        Log::log(LogLevel::Error, "Save", "failed {}", 42);
        Log::removeSink(sink);
    }

    const std::string content = readFile(path);
    EXPECT_NE(content.find("[ERROR]"), std::string::npos);
    EXPECT_NE(content.find("[Save]"), std::string::npos);
    EXPECT_NE(content.find("failed 42"), std::string::npos);

    std::filesystem::remove(path);
}

TEST_F(LogTests, FileSink_AppendMode_KeepsPreviousLines)
{
    const std::filesystem::path path = makeTempPath("append");
    {
        const std::shared_ptr<FileSink> sink = std::make_shared<FileSink>(path);
        Log::addSink(sink);
        Log::info("first");
        Log::clearSinks();
    }
    {
        const std::shared_ptr<FileSink> sink = std::make_shared<FileSink>(path, true);
        Log::addSink(sink);
        Log::info("second");
        Log::clearSinks();
    }

    const std::string content = readFile(path);
    EXPECT_NE(content.find("first"), std::string::npos);
    EXPECT_NE(content.find("second"), std::string::npos);
    EXPECT_EQ(std::count(content.begin(), content.end(), '\n'), 2);

    std::filesystem::remove(path);
}

TEST_F(LogTests, ThreadSafety_ConcurrentLogs_DeliverEveryMessageOnce)
{
    constexpr int kThreadCount = 8;
    constexpr int kMessagesPerThread = 200;

    RecordCollector collector;
    Log::addSink(makeSink(collector));

    std::atomic<int> ready{0};
    std::vector<std::thread> threads;
    threads.reserve(kThreadCount);
    for (int threadIndex = 0; threadIndex < kThreadCount; ++threadIndex) {
        threads.emplace_back([threadIndex, &ready]() {
            ready.fetch_add(1);
            while (ready.load() < kThreadCount) {
                std::this_thread::yield();
            }
            for (int messageIndex = 0; messageIndex < kMessagesPerThread; ++messageIndex) {
                Log::info("thread={} message={}", threadIndex, messageIndex);
            }
        });
    }
    for (std::thread& thread : threads) {
        thread.join();
    }

    const std::vector<LogRecord> records = collector.records();
    ASSERT_EQ(records.size(), static_cast<std::size_t>(kThreadCount * kMessagesPerThread));

    std::set<std::string> messages;
    for (const LogRecord& record : records) {
        messages.insert(record.message);
    }
    EXPECT_EQ(messages.size(), records.size());

    std::size_t missing = 0;
    for (int threadIndex = 0; threadIndex < kThreadCount; ++threadIndex) {
        for (int messageIndex = 0; messageIndex < kMessagesPerThread; ++messageIndex) {
            if (messages.count(std::format("thread={} message={}", threadIndex, messageIndex)) == 0) {
                ++missing;
            }
        }
    }
    EXPECT_EQ(missing, 0U);
}

TEST_F(LogTests, ThreadSafety_ConsoleSink_WritesWholeLines)
{
    constexpr int kThreadCount = 4;
    constexpr int kMessagesPerThread = 100;

    std::ostringstream stream;
    Log::addSink(std::make_shared<ConsoleSink>(stream));

    std::atomic<int> ready{0};
    std::vector<std::thread> threads;
    threads.reserve(kThreadCount);
    for (int threadIndex = 0; threadIndex < kThreadCount; ++threadIndex) {
        threads.emplace_back([threadIndex, &ready]() {
            ready.fetch_add(1);
            while (ready.load() < kThreadCount) {
                std::this_thread::yield();
            }
            for (int messageIndex = 0; messageIndex < kMessagesPerThread; ++messageIndex) {
                Log::info("thread={} message={}", threadIndex, messageIndex);
            }
        });
    }
    for (std::thread& thread : threads) {
        thread.join();
    }

    std::istringstream lines(stream.str());
    std::string line;
    std::size_t lineCount = 0;
    while (std::getline(lines, line)) {
        ++lineCount;
        EXPECT_EQ(line.rfind("[INFO] [General] ", 0), 0U);
    }
    EXPECT_EQ(lineCount, static_cast<std::size_t>(kThreadCount * kMessagesPerThread));
}
