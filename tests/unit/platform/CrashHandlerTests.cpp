#include "platform/CrashHandler.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <thread>

#include <gtest/gtest.h>

namespace {

using sprout::detail::CrashHandler;
using sprout::detail::CrashHandlerSettings;

std::filesystem::path makeTempDir(std::string_view name)
{
    static std::atomic<unsigned> counter{0};
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    std::filesystem::path path = std::filesystem::temp_directory_path()
        / ("sprout_crash_" + std::string(name) + "_" + std::to_string(stamp) + "_"
           + std::to_string(counter.fetch_add(1, std::memory_order_relaxed)));
    std::filesystem::create_directories(path);
    return path;
}

void removeDir(const std::filesystem::path& path)
{
    std::error_code ec;
    std::filesystem::remove_all(path, ec);
}

class CrashHandlerTests : public ::testing::Test {
protected:
    void SetUp() override
    {
        m_tempDir = makeTempDir("test");
        CrashHandler::shutdown();
    }

    void TearDown() override
    {
        CrashHandler::shutdown();
        removeDir(m_tempDir);
    }

    std::filesystem::path m_tempDir;
};

TEST_F(CrashHandlerTests, Initialize_ReturnsTrueOnFirstCall)
{
    CrashHandlerSettings settings;
    settings.dumpDirectory = m_tempDir.string();

    const bool result = CrashHandler::initialize(settings);

    EXPECT_TRUE(result);
    EXPECT_TRUE(CrashHandler::isInitialized());
}

TEST_F(CrashHandlerTests, Initialize_ReturnsFalseOnSecondCall)
{
    CrashHandlerSettings settings;
    settings.dumpDirectory = m_tempDir.string();

    ASSERT_TRUE(CrashHandler::initialize(settings));
    const bool result = CrashHandler::initialize(settings);

    EXPECT_FALSE(result);
    EXPECT_TRUE(CrashHandler::isInitialized());
}

TEST_F(CrashHandlerTests, Shutdown_ResetsState)
{
    CrashHandlerSettings settings;
    settings.dumpDirectory = m_tempDir.string();

    ASSERT_TRUE(CrashHandler::initialize(settings));
    CrashHandler::shutdown();

    EXPECT_FALSE(CrashHandler::isInitialized());
}

TEST_F(CrashHandlerTests, WriteDump_CreatesMinidumpFile)
{
    CrashHandlerSettings settings;
    settings.dumpDirectory = m_tempDir.string();

    ASSERT_TRUE(CrashHandler::initialize(settings));

    const std::string dumpPath = CrashHandler::writeDump("test_dump");

    EXPECT_FALSE(dumpPath.empty());
    EXPECT_TRUE(std::filesystem::exists(dumpPath));
    EXPECT_NE(std::filesystem::file_size(dumpPath), 0);

    const std::string lastPath = CrashHandler::lastDumpPath();
    EXPECT_EQ(lastPath, dumpPath);
}

TEST_F(CrashHandlerTests, WriteDump_ReturnsEmptyWhenNotInitialized)
{
    CrashHandler::shutdown();

    const std::string dumpPath = CrashHandler::writeDump("test");

    EXPECT_TRUE(dumpPath.empty());
}

TEST_F(CrashHandlerTests, SetLogBackupDirectory_SetsDirectory)
{
    const std::filesystem::path backupDir = m_tempDir / "LogBackups";
    CrashHandler::setLogBackupDirectory(backupDir.string());

    EXPECT_TRUE(std::filesystem::exists(backupDir));
}

TEST_F(CrashHandlerTests, BackupCurrentLog_CopiesLogFile)
{
    const std::filesystem::path logDir = m_tempDir / "Logs";
    std::filesystem::create_directories(logDir);

    const std::filesystem::path logFile = logDir / "test.log";
    {
        std::ofstream ofs(logFile);
        ofs << "[INFO] [General] Engine started\n";
        ofs << "[WARNING] [Physics] High velocity detected\n";
        ofs << "[ERROR] [Graphics] Shader compilation failed\n";
    }

    CrashHandlerSettings settings;
    settings.dumpDirectory = m_tempDir.string();
    settings.maxLogBackupSize = 0;
    ASSERT_TRUE(CrashHandler::initialize(settings));

    const std::string backupPath = CrashHandler::backupCurrentLog("manual_backup");

    EXPECT_FALSE(backupPath.empty());
    EXPECT_TRUE(std::filesystem::exists(backupPath));

    std::ifstream ifs(backupPath);
    std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    EXPECT_NE(content.find("Engine started"), std::string::npos);
    EXPECT_NE(content.find("High velocity detected"), std::string::npos);
    EXPECT_NE(content.find("Shader compilation failed"), std::string::npos);
}

TEST_F(CrashHandlerTests, BackupCurrentLog_RespectsMaxSize)
{
    const std::filesystem::path logDir = m_tempDir / "Logs";
    std::filesystem::create_directories(logDir);

    const std::filesystem::path logFile = logDir / "large.log";
    {
        std::ofstream ofs(logFile);
        for (int i = 0; i < 1000; ++i) {
            ofs << "[INFO] [General] Line " << i << " with some data to make it longer\n";
        }
    }

    CrashHandlerSettings settings;
    settings.dumpDirectory = m_tempDir.string();
    settings.maxLogBackupSize = 500;
    ASSERT_TRUE(CrashHandler::initialize(settings));

    const std::string backupPath = CrashHandler::backupCurrentLog("size_test");

    EXPECT_FALSE(backupPath.empty());
    EXPECT_TRUE(std::filesystem::exists(backupPath));
    EXPECT_LE(std::filesystem::file_size(backupPath), 600);
}

TEST_F(CrashHandlerTests, BackupCurrentLog_ReturnsEmptyWhenNoLog)
{
    CrashHandlerSettings settings;
    settings.dumpDirectory = m_tempDir.string();
    ASSERT_TRUE(CrashHandler::initialize(settings));

    const std::string backupPath = CrashHandler::backupCurrentLog("no_log");

    EXPECT_TRUE(backupPath.empty());
}

TEST_F(CrashHandlerTests, OnCrashCallback_IsInvoked)
{
    std::string capturedPath;
    std::mutex callbackMutex;

    CrashHandlerSettings settings;
    settings.dumpDirectory = m_tempDir.string();
    settings.onCrash = [&capturedPath, &callbackMutex](std::string_view path) {
        std::lock_guard lock(callbackMutex);
        capturedPath = path;
    };

    ASSERT_TRUE(CrashHandler::initialize(settings));
    const std::string dumpPath = CrashHandler::writeDump("callback_test");

    EXPECT_FALSE(dumpPath.empty());
    EXPECT_EQ(capturedPath, dumpPath);
}

TEST_F(CrashHandlerTests, DumpDirectory_RelativePath_IsResolvedToExecutableDir)
{
    CrashHandlerSettings settings;
    settings.dumpDirectory = "RelativeDumps";

    ASSERT_TRUE(CrashHandler::initialize(settings));
    const std::string dumpPath = CrashHandler::writeDump("relative_test");

    EXPECT_FALSE(dumpPath.empty());
    EXPECT_TRUE(std::filesystem::exists(dumpPath));
    EXPECT_NE(dumpPath.find("RelativeDumps"), std::string::npos);

    std::filesystem::path exeDir = std::filesystem::current_path();
    std::filesystem::path relativeDumpDir = exeDir / "RelativeDumps";
    std::filesystem::remove_all(relativeDumpDir);
}

TEST_F(CrashHandlerTests, LastDumpPath_ReturnsEmptyBeforeAnyDump)
{
    CrashHandlerSettings settings;
    settings.dumpDirectory = m_tempDir.string();
    ASSERT_TRUE(CrashHandler::initialize(settings));

    const std::string lastPath = CrashHandler::lastDumpPath();

    EXPECT_TRUE(lastPath.empty());
}

}  // namespace