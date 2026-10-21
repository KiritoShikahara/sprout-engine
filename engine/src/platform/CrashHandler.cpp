#include "platform/CrashHandler.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")
#endif

namespace sprout::detail {

namespace {

struct CrashHandlerState {
    std::mutex mutex;
    CrashHandlerSettings settings;
    std::string lastDumpPath;
    std::string logBackupDirectory;
    std::atomic<bool> initialized{false};
    std::atomic<bool> inCrashHandler{false};
    std::thread::id mainThreadId{};
    PTOP_LEVEL_EXCEPTION_FILTER previousFilter{nullptr};
};

CrashHandlerState& state() noexcept
{
    static CrashHandlerState instance;
    return instance;
}

std::string getTimestamp() noexcept
{
    const auto now = std::chrono::system_clock::now();
    const auto timeT = std::chrono::system_clock::to_time_t(now);
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &timeT);
#else
    localtime_r(&timeT, &tm);
#endif

    char buffer[64];
    std::snprintf(buffer, sizeof(buffer), "%04d%02d%02d_%02d%02d%02d_%03lld",
        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
        tm.tm_hour, tm.tm_min, tm.tm_sec,
        static_cast<long long>(ms.count()));
    return buffer;
}

std::string getExecutableDirectory() noexcept
{
#if defined(_WIN32)
    wchar_t path[MAX_PATH];
    const DWORD len = ::GetModuleFileNameW(nullptr, path, MAX_PATH);
    if (len == 0 || len == MAX_PATH) {
        return ".";
    }
    std::wstring wpath(path, len);
    const auto pos = wpath.find_last_of(L'\\');
    if (pos == std::wstring::npos) {
        return ".";
    }
    std::string result;
    result.resize(pos);
    const int size = ::WideCharToMultiByte(CP_UTF8, 0, wpath.c_str(), static_cast<int>(pos), result.data(), static_cast<int>(result.size()), nullptr, nullptr);
    if (size > 0) {
        result.resize(size);
    }
    return result;
#else
    return ".";
#endif
}

std::string getDumpDirectory(const CrashHandlerSettings& settings) noexcept
{
    if (!settings.dumpDirectory.empty()) {
        std::filesystem::path path(settings.dumpDirectory);
        if (!path.is_absolute()) {
            path = std::filesystem::path(getExecutableDirectory()) / path;
        }
        std::error_code ec;
        std::filesystem::create_directories(path, ec);
        std::string result = path.string();
        std::replace(result.begin(), result.end(), '\\', '/');
        return result;
    }
    return getExecutableDirectory();
}

std::string generateDumpPath(const std::string& dumpDir, std::string_view comment) noexcept
{
    std::string filename = "crash_" + getTimestamp();
    if (!comment.empty()) {
        filename += "_";
        filename += comment;
        std::replace(filename.begin(), filename.end(), ' ', '_');
        std::replace(filename.begin(), filename.end(), '/', '_');
        std::replace(filename.begin(), filename.end(), '\\', '_');
    }
    filename += ".dmp";
    return (std::filesystem::path(dumpDir) / filename).string();
}

std::string getLogBackupDirectory() noexcept
{
    CrashHandlerState& s = state();
    std::lock_guard lock(s.mutex);
    if (!s.logBackupDirectory.empty()) {
        return s.logBackupDirectory;
    }
    const std::string dumpDir = getDumpDirectory(s.settings);
    return (std::filesystem::path(dumpDir) / "Logs").string();
}

void ensureDirectoryExists(const std::string& path) noexcept
{
    std::error_code ec;
    std::filesystem::create_directories(path, ec);
}

std::string findCurrentLogFile() noexcept
{
    std::string executableDir = getExecutableDirectory();
    std::filesystem::path logDir = executableDir / "Logs";
    if (!std::filesystem::exists(logDir)) {
        logDir = executableDir;
    }

    std::string latestLog;
    std::filesystem::file_time_type latestTime = std::filesystem::file_time_type::min();

    std::error_code ec;
    for (const auto& entry : std::filesystem::directory_iterator(logDir, ec)) {
        if (ec) break;
        if (!entry.is_regular_file()) continue;
        const std::string ext = entry.path().extension().string();
        if (ext != ".log" && ext != ".txt") continue;
        const auto ftime = entry.last_write_time(ec);
        if (ec) continue;
        if (ftime > latestTime) {
            latestTime = ftime;
            latestLog = entry.path().string();
        }
    }
    return latestLog;
}

std::string copyFileWithLimit(const std::string& sourcePath, const std::string& destPath, std::size_t maxSize) noexcept
{
    std::error_code ec;
    const std::uintmax_t sourceSize = std::filesystem::file_size(sourcePath, ec);
    if (ec) {
        return {};
    }

    if (maxSize > 0 && sourceSize > maxSize) {
        std::ifstream input(sourcePath, std::ios::binary);
        std::ofstream output(destPath, std::ios::binary);
        if (!input || !output) {
            return {};
        }
        input.seekg(static_cast<std::streamoff>(sourceSize) - static_cast<std::streamoff>(maxSize));
        if (input.fail()) {
            input.seekg(0);
        }
        output << input.rdbuf();
        return destPath;
    }

    std::filesystem::copy_file(sourcePath, destPath, std::filesystem::copy_options::overwrite_existing, ec);
    if (ec) {
        return {};
    }
    return destPath;
}

#if defined(_WIN32)

LONG WINAPI crashExceptionFilter(EXCEPTION_POINTERS* exceptionInfo) noexcept
{
    CrashHandlerState& s = state();

    if (s.inCrashHandler.exchange(true)) {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    if (std::this_thread::get_id() != s.mainThreadId) {
        s.inCrashHandler.store(false);
        return EXCEPTION_CONTINUE_SEARCH;
    }

    std::string dumpPath;
    try {
        const std::string dumpDir = getDumpDirectory(s.settings);
        ensureDirectoryExists(dumpDir);

        dumpPath = generateDumpPath(dumpDir, "unhandled_exception");

        HANDLE fileHandle = ::CreateFileW(
            std::filesystem::path(dumpPath).wstring().c_str(),
            GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

        if (fileHandle != INVALID_HANDLE_VALUE) {
            MINIDUMP_EXCEPTION_INFORMATION exceptionParam{};
            exceptionParam.ThreadId = ::GetCurrentThreadId();
            exceptionParam.ExceptionPointers = exceptionInfo;
            exceptionParam.ClientPointers = TRUE;

            const MINIDUMP_TYPE dumpType = static_cast<MINIDUMP_TYPE>(
                MiniDumpWithIndirectlyReferencedMemory |
                MiniDumpScanMemory |
                MiniDumpWithProcessThreadData |
                MiniDumpWithThreadInfo |
                MiniDumpWithHandleData |
                MiniDumpWithUnloadedModules);

            const BOOL success = ::MiniDumpWriteDump(
                ::GetCurrentProcess(), ::GetCurrentProcessId(), fileHandle,
                dumpType, &exceptionParam, nullptr, nullptr);

            ::CloseHandle(fileHandle);

            if (!success) {
                std::filesystem::remove(dumpPath);
                dumpPath.clear();
            }
        } else {
            dumpPath.clear();
        }
    } catch (...) {
        dumpPath.clear();
    }

    if (!dumpPath.empty()) {
        std::lock_guard lock(s.mutex);
        s.lastDumpPath = dumpPath;

        if (s.settings.onCrash) {
            try {
                s.settings.onCrash(dumpPath);
            } catch (...) {
            }
        }
    }

    std::string logBackupDir = getLogBackupDirectory();
    ensureDirectoryExists(logBackupDir);

    const std::string currentLog = findCurrentLogFile();
    if (!currentLog.empty()) {
        const std::string backupPath = (std::filesystem::path(logBackupDir) / ("log_" + getTimestamp() + ".log")).string();
        copyFileWithLimit(currentLog, backupPath, s.settings.maxLogBackupSize);
    }

    if (s.previousFilter) {
        return s.previousFilter(exceptionInfo);
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

#else

void crashExceptionFilter() noexcept
{
}

#endif

}  // namespace

bool CrashHandler::initialize(const CrashHandlerSettings& settings) noexcept
{
    CrashHandlerState& s = state();
    std::lock_guard lock(s.mutex);

    if (s.initialized.load()) {
        return false;
    }

    s.settings = settings;
    s.mainThreadId = std::this_thread::get_id();

#if defined(_WIN32)
    s.previousFilter = ::SetUnhandledExceptionFilter(&crashExceptionFilter);
    if (s.previousFilter == nullptr) {
        return false;
    }
#endif

    s.initialized.store(true);
    return true;
}

bool CrashHandler::isInitialized() noexcept
{
    return state().initialized.load();
}

std::string CrashHandler::lastDumpPath() noexcept
{
    CrashHandlerState& s = state();
    std::lock_guard lock(s.mutex);
    return s.lastDumpPath;
}

std::string CrashHandler::writeDump(std::string_view comment) noexcept
{
#if defined(_WIN32)
    CrashHandlerState& s = state();
    std::lock_guard lock(s.mutex);

    if (!s.initialized.load()) {
        return {};
    }

    const std::string dumpDir = getDumpDirectory(s.settings);
    ensureDirectoryExists(dumpDir);

    const std::string dumpPath = generateDumpPath(dumpDir, comment);

    HANDLE fileHandle = ::CreateFileW(
        std::filesystem::path(dumpPath).wstring().c_str(),
        GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (fileHandle == INVALID_HANDLE_VALUE) {
        return {};
    }

    EXCEPTION_POINTERS exceptionInfo{};
    EXCEPTION_RECORD exceptionRecord{};
    exceptionRecord.ExceptionCode = EXCEPTION_BREAKPOINT;
    exceptionRecord.ExceptionAddress = _ReturnAddress();
    exceptionInfo.ExceptionRecord = &exceptionRecord;

    CONTEXT context{};
    ::RtlCaptureContext(&context);
    exceptionInfo.ContextRecord = &context;

    MINIDUMP_EXCEPTION_INFORMATION exceptionParam{};
    exceptionParam.ThreadId = ::GetCurrentThreadId();
    exceptionParam.ExceptionPointers = &exceptionInfo;
    exceptionParam.ClientPointers = TRUE;

    const MINIDUMP_TYPE dumpType = static_cast<MINIDUMP_TYPE>(
        MiniDumpWithIndirectlyReferencedMemory |
        MiniDumpScanMemory |
        MiniDumpWithProcessThreadData |
        MiniDumpWithThreadInfo |
        MiniDumpWithHandleData |
        MiniDumpWithUnloadedModules);

    const BOOL success = ::MiniDumpWriteDump(
        ::GetCurrentProcess(), ::GetCurrentProcessId(), fileHandle,
        dumpType, &exceptionParam, nullptr, nullptr);

    ::CloseHandle(fileHandle);

    if (!success) {
        std::filesystem::remove(dumpPath);
        return {};
    }

    s.lastDumpPath = dumpPath;
    return dumpPath;
#else
    (void)comment;
    return {};
#endif
}

void CrashHandler::setLogBackupDirectory(std::string_view directory) noexcept
{
    CrashHandlerState& s = state();
    std::lock_guard lock(s.mutex);
    s.logBackupDirectory = directory;
    ensureDirectoryExists(s.logBackupDirectory);
}

std::string CrashHandler::backupCurrentLog(std::string_view reason) noexcept
{
    CrashHandlerState& s = state();
    std::lock_guard lock(s.mutex);

    const std::string logBackupDir = getLogBackupDirectory();
    ensureDirectoryExists(logBackupDir);

    const std::string currentLog = findCurrentLogFile();
    if (currentLog.empty()) {
        return {};
    }

    std::string reasonStr(reason);
    std::replace(reasonStr.begin(), reasonStr.end(), ' ', '_');
    std::replace(reasonStr.begin(), reasonStr.end(), '/', '_');
    std::replace(reasonStr.begin(), reasonStr.end(), '\\', '_');

    const std::string backupPath = (std::filesystem::path(logBackupDir) / ("log_" + getTimestamp() + "_" + reasonStr + ".log")).string();
    return copyFileWithLimit(currentLog, backupPath, s.settings.maxLogBackupSize);
}

void CrashHandler::shutdown() noexcept
{
    CrashHandlerState& s = state();
    std::lock_guard lock(s.mutex);

    if (!s.initialized.load()) {
        return;
    }

#if defined(_WIN32)
    if (s.previousFilter) {
        ::SetUnhandledExceptionFilter(s.previousFilter);
        s.previousFilter = nullptr;
    }
#endif

    s.initialized.store(false);
    s.inCrashHandler.store(false);
    s.lastDumpPath.clear();
}

}  // namespace sprout::detail