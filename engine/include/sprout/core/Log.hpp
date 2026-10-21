#pragma once

#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace sprout {

/// @brief ログの重大度。
enum class LogLevel {
    Trace,
    Info,
    Warning,
    Error,
    Fatal,
};

/// @brief 1 件のログ。
struct LogRecord {
    /// 重大度。
    LogLevel level{LogLevel::Info};

    /// カテゴリ（"Physics" など）。
    std::string category;

    /// 書式化済みメッセージ。
    std::string message;
};

/// @brief レベル名（"TRACE" など）を返す。
/// @param level 対象のレベル。
/// @return 大文字のレベル名。未知の値は "UNKNOWN"。
[[nodiscard]] std::string_view toString(LogLevel level) noexcept;

/// @brief シンク出力用の 1 行文字列を返す（末尾改行なし）。
/// @param record 対象のログ。
/// @return "[LEVEL] [category] message" 形式の文字列。
[[nodiscard]] std::string formatLogRecord(const LogRecord& record);

/// @brief ログの出力先。
///
/// write() は Log から複数スレッドで呼ばれうるため、実装側で排他する。
class LogSink {
public:
    virtual ~LogSink() = default;

    /// @brief 1 件のログを書き出す。
    /// @param record 書き出すログ。
    virtual void write(const LogRecord& record) = 0;

    /// @brief 保留中の出力を確定する。
    virtual void flush() {}
};

/// @brief ストリーム（既定は標準出力）へ書き出すシンク。
class ConsoleSink final : public LogSink {
public:
    /// @brief 出力先ストリームを指定して生成する。
    /// @param stream 出力先。呼び出し側が生存期間を管理する。
    explicit ConsoleSink(std::ostream& stream = std::cout);

    void write(const LogRecord& record) override;
    void flush() override;

private:
    std::ostream& m_stream;
    mutable std::mutex m_mutex;
};

/// @brief ファイルへ書き出すシンク。1 件ごとに改行してフラッシュする。
class FileSink final : public LogSink {
public:
    /// @brief ファイルを開いて生成する。
    /// @param path 出力先ファイル。親ディレクトリは存在している必要がある。
    /// @param append true なら追記、false なら切り詰める。
    explicit FileSink(std::filesystem::path path, bool append = false);

    void write(const LogRecord& record) override;
    void flush() override;

    /// @brief ファイルを開けているか返す。
    /// @return 開けていれば true。
    [[nodiscard]] bool isOpen() const;

private:
    std::ofstream m_stream;
    mutable std::mutex m_mutex;
};

/// @brief コールバックへ書き出すシンク。
class CallbackSink final : public LogSink {
public:
    /// @brief ログを受け取る関数の型。
    using Callback = std::function<void(const LogRecord&)>;

    /// @brief コールバックを指定して生成する。
    /// @param callback ログごとに呼ばれる関数。空なら何もしない。
    explicit CallbackSink(Callback callback);

    void write(const LogRecord& record) override;

private:
    Callback m_callback;
    std::recursive_mutex m_mutex;
};

/// @brief レベル・カテゴリ・複数シンクを持つスレッド安全なログ。
///
/// 書式は std::format と同じ。シンクを追加するまで出力は行われない。
/// いずれかのシンクが例外を投げても他のシンクとログ呼び出しには影響しない。
///
/// @code
/// sprout::Log::addSink(std::make_shared<sprout::ConsoleSink>());
/// sprout::Log::info("loaded {}", path);
/// sprout::Log::log(sprout::LogLevel::Warning, "Physics", "body {} missing", id);
/// @endcode
class Log {
public:
    /// カテゴリ未指定時に使うカテゴリ名。
    static constexpr std::string_view kDefaultCategory = "General";

    Log() = delete;

    /// @brief シンクを追加する。null は無視する。
    static void addSink(std::shared_ptr<LogSink> sink);

    /// @brief 同一のシンクをすべて取り除く。
    static void removeSink(const std::shared_ptr<LogSink>& sink);

    /// @brief すべてのシンクを取り除く。
    static void clearSinks();

    /// @brief 出力する最小レベルを設定する。
    static void setMinimumLevel(LogLevel level) noexcept;

    /// @brief 出力する最小レベルを返す。
    [[nodiscard]] static LogLevel minimumLevel() noexcept;

    /// @brief カテゴリ単位の有効/無効を設定する。
    static void setCategoryEnabled(std::string_view category, bool enabled);

    /// @brief カテゴリが有効か返す。未設定のカテゴリは有効。
    [[nodiscard]] static bool isCategoryEnabled(std::string_view category);

    /// @brief カテゴリ設定をすべて削除する（全カテゴリ有効に戻す）。
    static void clearCategoryFilters();

    /// @brief Trace レベルで出力する。
    template <typename... Args>
    static void trace(std::format_string<Args...> fmt, Args&&... args)
    {
        write(LogLevel::Trace, kDefaultCategory, fmt, std::forward<Args>(args)...);
    }

    /// @brief Info レベルで出力する。
    template <typename... Args>
    static void info(std::format_string<Args...> fmt, Args&&... args)
    {
        write(LogLevel::Info, kDefaultCategory, fmt, std::forward<Args>(args)...);
    }

    /// @brief Warning レベルで出力する。
    template <typename... Args>
    static void warning(std::format_string<Args...> fmt, Args&&... args)
    {
        write(LogLevel::Warning, kDefaultCategory, fmt, std::forward<Args>(args)...);
    }

    /// @brief Error レベルで出力する。
    template <typename... Args>
    static void error(std::format_string<Args...> fmt, Args&&... args)
    {
        write(LogLevel::Error, kDefaultCategory, fmt, std::forward<Args>(args)...);
    }

    /// @brief Fatal レベルで出力する。
    template <typename... Args>
    static void fatal(std::format_string<Args...> fmt, Args&&... args)
    {
        write(LogLevel::Fatal, kDefaultCategory, fmt, std::forward<Args>(args)...);
    }

    /// @brief レベルとカテゴリを指定して出力する。
    template <typename... Args>
    static void log(LogLevel level, std::string_view category, std::format_string<Args...> fmt, Args&&... args)
    {
        write(level, category, fmt, std::forward<Args>(args)...);
    }

private:
    template <typename... Args>
    static void write(LogLevel level, std::string_view category, std::format_string<Args...> fmt, Args&&... args)
    {
        try {
            if (!isEnabled(level, category)) {
                return;
            }
            emit(LogRecord{level, std::string(category), std::format(fmt, std::forward<Args>(args)...)});
        } catch (...) {
        }
    }

    [[nodiscard]] static bool isEnabled(LogLevel level, std::string_view category);
    static void emit(const LogRecord& record);
};

}  // namespace sprout
