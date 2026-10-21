#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <span>

namespace sprout {

namespace detail {
struct JobState;
}

/// @brief 投入済みジョブの完了状態を参照する軽量ハンドル。
///
/// コピー可能な値型。既定構築したハンドルは無効で、どのジョブにも対応しない。
/// ハンドルを破棄してもジョブはキャンセルされない。
///
/// @code
/// sprout::JobSystem jobs;
/// const sprout::JobHandle handle = jobs.submit([]{ /* 重い処理 */ });
/// jobs.wait(handle);
/// @endcode
class JobHandle {
public:
    /// @brief 無効なハンドルを生成する。
    JobHandle() noexcept = default;

    /// @brief 有効なジョブを参照しているか判定する。
    /// @return 投入済みジョブを参照していれば true。
    [[nodiscard]] bool isValid() const noexcept;

    /// @brief 有効なジョブを参照しているか判定する。
    /// @return 投入済みジョブを参照していれば true。
    explicit operator bool() const noexcept;

    /// @brief ジョブが実行を終えたか判定する。
    /// @return 完了（キャンセルを含む）していれば true。無効なハンドルは false。
    [[nodiscard]] bool isComplete() const;

    /// @brief ジョブの完了を待つ。
    ///
    /// 無効なハンドルでは即座に戻る。呼び出したスレッドをブロックする。
    void wait() const;

private:
    explicit JobHandle(std::shared_ptr<detail::JobState> state) noexcept;

    std::shared_ptr<detail::JobState> m_state;

    friend class JobSystem;
};

/// @brief 固定数のワーカースレッドでジョブを実行するスケジューラ。
///
/// ジョブは submit() で投入し、依存関係を指定すると前提ジョブの完了後に実行される。
/// メインスレッドで実行したい処理は postMainThread() で積み、毎フレーム
/// pumpMainThread() を呼んで実行する。stop() は未実行のジョブを破棄して
/// ワーカーを停止する。全てのメンバーはスレッド安全。
class JobSystem {
public:
    /// @brief ワーカーで実行するジョブ。
    using JobFunction = std::function<void()>;

    /// @brief メインスレッドで実行する処理。
    using MainThreadFunction = std::function<void()>;

    /// @brief ワーカースレッドを起動する。
    /// @param workerCount ワーカー数。0 なら defaultWorkerCount() を使う。
    explicit JobSystem(std::size_t workerCount = 0);

    /// @brief 実行中のジョブの完了を待ち、ワーカーを停止する。
    ~JobSystem();

    JobSystem(const JobSystem&) = delete;
    JobSystem& operator=(const JobSystem&) = delete;
    JobSystem(JobSystem&&) = delete;
    JobSystem& operator=(JobSystem&&) = delete;

    /// @brief ジョブを投入する。
    /// @param job 実行する処理。空の場合は無効なハンドルを返す。
    /// @param dependencies 前提ジョブ。全て完了してから実行される。
    /// @return 投入したジョブのハンドル。停止後の投入は無効なハンドルになる。
    [[nodiscard]] JobHandle submit(JobFunction job, std::span<const JobHandle> dependencies = {});

    /// @brief メインスレッドで実行する処理を積む。
    ///
    /// ワーカーからも呼べる。実行は pumpMainThread() を呼んだスレッドで行われる。
    /// 停止後は無視される。
    /// @param callback 実行する処理。
    void postMainThread(MainThreadFunction callback);

    /// @brief 積まれたメインスレッド処理を実行する。
    /// @return 実行した件数。停止後は 0。
    std::size_t pumpMainThread();

    /// @brief ジョブの完了を待つ。
    /// @param handle 対象のジョブ。無効なら即座に戻る。
    void wait(const JobHandle& handle);

    /// @brief 投入済みの全ジョブの完了を待つ。
    ///
    /// メインスレッド処理は対象に含まない。
    void waitAll();

    /// @brief 未実行のジョブを破棄してワーカーを停止する。
    ///
    /// 実行中のジョブは完了を待つ。冪等。
    void stop();

    /// @brief ワーカーが稼働中か判定する。
    /// @return stop() 後なら false。
    [[nodiscard]] bool isRunning() const noexcept;

    /// @brief 設定したワーカー数を返す。
    /// @return ワーカー数。
    [[nodiscard]] std::size_t workerCount() const noexcept;

    /// @brief 標準のワーカー数を返す。
    /// @return ハードウェア並列数。取得できなければ 1。
    [[nodiscard]] static std::size_t defaultWorkerCount() noexcept;

private:
    class Impl;

    std::unique_ptr<Impl> m_impl;
};

}  // namespace sprout
