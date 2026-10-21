#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <string_view>

namespace sprout::detail {

/// @brief ミニダンプ生成時の設定。
struct CrashHandlerSettings {
    /// @brief ミニダンプを出力するディレクトリ。空なら実行ファイルと同じ場所。
    std::string dumpDirectory{};

    /// @brief 退避するログファイルの最大サイズ（バイト）。0 なら無制限。
    std::size_t maxLogBackupSize{10 * 1024 * 1024};

    /// @brief クラッシュ時に呼び出されるコールバック。ログ退避や UI 通知に使う。
    std::function<void(std::string_view dumpPath)> onCrash{};
};

/// @brief クラッシュハンドラ。
///
/// Windows SEH と MiniDumpWriteDump を使って、未捕捉の例外（アクセス違反等）が発生した
/// ときにミニダンプを生成し、ログを退避する。
///
/// 早期に `initialize()` を呼ぶこと。`main()` の最初で呼ぶのが望ましい。
///
/// @code
/// sprout::detail::CrashHandlerSettings settings;
/// settings.dumpDirectory = "CrashDumps";
/// settings.onCrash = [](std::string_view path) { std::cerr << "Crash dump: " << path << '\n'; };
/// sprout::detail::CrashHandler::initialize(settings);
/// @endcode
class CrashHandler {
public:
    /// @brief クラッシュハンドラを初期化する。
    /// @param settings 設定。初回呼び出しのみ有効。
    /// @return 初期化に成功したら true。既に初期化済みなら false。
    static bool initialize(const CrashHandlerSettings& settings) noexcept;

    /// @brief クラッシュハンドラが初期化済みか判定する。
    /// @return 初期化済みなら true。
    [[nodiscard]] static bool isInitialized() noexcept;

    /// @brief 最後に生成されたミニダンプのパスを返す。
    /// @return パス。生成されていなければ空文字列。
    [[nodiscard]] static std::string lastDumpPath() noexcept;

    /// @brief 現在のプロセスでミニダンプを手動生成する（テスト・デバッグ用）。
    /// @param comment ダンプに含めるコメント。
    /// @return 生成されたダンプのパス。失敗なら空文字列。
    [[nodiscard]] static std::string writeDump(std::string_view comment = {}) noexcept;

    /// @brief ログ退避用のディレクトリを設定する。
    /// @param directory 退避先ディレクトリ。
    static void setLogBackupDirectory(std::string_view directory) noexcept;

    /// @brief 現在のログファイルを退避させる。
    /// @param reason 退避理由（ファイル名に含める）。
    /// @return 退避先ファイルのパス。失敗なら空文字列。
    [[nodiscard]] static std::string backupCurrentLog(std::string_view reason) noexcept;

    /// @brief クラッシュハンドラを無効化する（テスト・シャットダウン用）。
    static void shutdown() noexcept;

    CrashHandler() = delete;
    CrashHandler(const CrashHandler&) = delete;
    CrashHandler& operator=(const CrashHandler&) = delete;
};

}  // namespace sprout::detail