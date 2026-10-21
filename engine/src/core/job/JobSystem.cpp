#include "core/job/JobHandle.hpp"

#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

namespace sprout::detail {

struct JobState {
    std::function<void()> fn;
    std::mutex mutex;
    std::condition_variable completed;
    std::vector<std::shared_ptr<JobState>> dependents;
    std::atomic<std::size_t> pendingDependencies{0};
    bool complete{false};
};

}  // namespace sprout::detail

namespace sprout {

class JobSystem::Impl {
public:
    explicit Impl(std::size_t count);

    ~Impl();

    void stop();
    void waitAll();
    void workerLoop();

private:
    friend class JobSystem;

    void run(const std::shared_ptr<detail::JobState>& state);
    void finish(const std::shared_ptr<detail::JobState>& state);
    void cancelReadyQueue();

    std::mutex stopMutex;
    std::mutex mutex;
    std::condition_variable cv;
    std::deque<std::shared_ptr<detail::JobState>> ready;
    std::vector<std::thread> threads;
    std::size_t workerCount{0};
    std::atomic<bool> running{true};
    bool stopping{false};
    std::atomic<std::size_t> outstanding{0};

    std::mutex mainMutex;
    std::deque<std::function<void()>> mainQueue;

    std::mutex waitMutex;
    std::condition_variable waitCv;
};

JobSystem::Impl::Impl(std::size_t count)
    : workerCount{count == 0 ? JobSystem::defaultWorkerCount() : count}
{
    try {
        for (std::size_t index = 0; index < workerCount; ++index) {
            threads.emplace_back([this] { workerLoop(); });
        }
    } catch (...) {
        stop();
    }
}

JobSystem::Impl::~Impl()
{
    stop();
}

void JobSystem::Impl::stop()
{
    const std::lock_guard stopLock(stopMutex);

    {
        const std::lock_guard lock(mutex);
        stopping = true;
        running.store(false);
    }
    cv.notify_all();

    for (std::thread& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    threads.clear();

    cancelReadyQueue();

    const std::lock_guard mainLock(mainMutex);
    mainQueue.clear();
}

void JobSystem::Impl::waitAll()
{
    std::unique_lock lock(waitMutex);
    waitCv.wait(lock, [this] { return outstanding.load() == 0U; });
}

void JobSystem::Impl::workerLoop()
{
    std::unique_lock lock(mutex);

    for (;;) {
        cv.wait(lock, [this] { return stopping || !ready.empty(); });

        if (stopping) {
            std::deque<std::shared_ptr<detail::JobState>> pending;
            pending.swap(ready);
            lock.unlock();
            for (const std::shared_ptr<detail::JobState>& state : pending) {
                finish(state);
            }
            lock.lock();
            if (ready.empty()) {
                return;
            }
            continue;
        }

        const std::shared_ptr<detail::JobState> state = std::move(ready.front());
        ready.pop_front();
        lock.unlock();

        run(state);
        finish(state);

        lock.lock();
    }
}

void JobSystem::Impl::run(const std::shared_ptr<detail::JobState>& state)
{
    if (!state->fn) {
        return;
    }

    try {
        state->fn();
    } catch (...) {
    }
}

void JobSystem::Impl::finish(const std::shared_ptr<detail::JobState>& state)
{
    std::vector<std::shared_ptr<detail::JobState>> dependents;
    {
        const std::lock_guard lock(state->mutex);
        if (state->complete) {
            return;
        }
        state->complete = true;
        dependents.swap(state->dependents);
        state->fn = nullptr;
    }
    state->completed.notify_all();

    {
        const std::lock_guard lock(mutex);
        for (const std::shared_ptr<detail::JobState>& dependent : dependents) {
            if (dependent->pendingDependencies.fetch_sub(1, std::memory_order_acq_rel) == 1U) {
                ready.push_back(dependent);
            }
        }
    }
    cv.notify_all();

    if (outstanding.fetch_sub(1, std::memory_order_acq_rel) == 1U) {
        const std::lock_guard lock(waitMutex);
        waitCv.notify_all();
    }
}

void JobSystem::Impl::cancelReadyQueue()
{
    for (;;) {
        std::deque<std::shared_ptr<detail::JobState>> pending;
        {
            const std::lock_guard lock(mutex);
            if (ready.empty()) {
                return;
            }
            pending.swap(ready);
        }
        for (const std::shared_ptr<detail::JobState>& state : pending) {
            finish(state);
        }
    }
}

JobHandle::JobHandle(std::shared_ptr<detail::JobState> state) noexcept
    : m_state{std::move(state)}
{
}

bool JobHandle::isValid() const noexcept
{
    return m_state != nullptr;
}

JobHandle::operator bool() const noexcept
{
    return isValid();
}

bool JobHandle::isComplete() const
{
    if (!m_state) {
        return false;
    }

    const std::lock_guard lock(m_state->mutex);
    return m_state->complete;
}

void JobHandle::wait() const
{
    if (!m_state) {
        return;
    }

    std::unique_lock lock(m_state->mutex);
    m_state->completed.wait(lock, [this] { return m_state->complete; });
}

JobSystem::JobSystem(std::size_t workerCount)
    : m_impl{std::make_unique<Impl>(workerCount)}
{
}

JobSystem::~JobSystem()
{
    stop();
}

JobHandle JobSystem::submit(JobFunction job, std::span<const JobHandle> dependencies)
{
    if (!job || !m_impl) {
        return {};
    }

    const std::shared_ptr<detail::JobState> state = std::make_shared<detail::JobState>();
    state->fn = std::move(job);

    std::size_t waiting = 0;
    {
        const std::lock_guard lock(m_impl->mutex);
        if (!m_impl->running.load()) {
            return {};
        }

        for (const JobHandle& dependency : dependencies) {
            if (!dependency.m_state) {
                continue;
            }

            const std::lock_guard dependencyLock(dependency.m_state->mutex);
            if (dependency.m_state->complete) {
                continue;
            }
            dependency.m_state->dependents.push_back(state);
            ++waiting;
        }

        state->pendingDependencies.store(waiting);
        m_impl->outstanding.fetch_add(1, std::memory_order_acq_rel);

        if (waiting == 0U) {
            m_impl->ready.push_back(state);
            m_impl->cv.notify_one();
        }
    }

    return JobHandle{state};
}

void JobSystem::postMainThread(MainThreadFunction callback)
{
    if (!callback || !m_impl) {
        return;
    }

    const std::lock_guard lock(m_impl->mainMutex);
    if (!m_impl->running.load()) {
        return;
    }
    m_impl->mainQueue.push_back(std::move(callback));
}

std::size_t JobSystem::pumpMainThread()
{
    if (!m_impl) {
        return 0;
    }

    std::deque<MainThreadFunction> callbacks;
    {
        const std::lock_guard lock(m_impl->mainMutex);
        if (!m_impl->running.load()) {
            return 0;
        }
        callbacks.swap(m_impl->mainQueue);
    }

    std::size_t executed = 0;
    for (MainThreadFunction& callback : callbacks) {
        try {
            callback();
        } catch (...) {
        }
        ++executed;
    }
    return executed;
}

void JobSystem::wait(const JobHandle& handle)
{
    handle.wait();
}

void JobSystem::waitAll()
{
    if (!m_impl) {
        return;
    }
    m_impl->waitAll();
}

void JobSystem::stop()
{
    if (!m_impl) {
        return;
    }
    m_impl->stop();
}

bool JobSystem::isRunning() const noexcept
{
    return m_impl && m_impl->running.load();
}

std::size_t JobSystem::workerCount() const noexcept
{
    return m_impl ? m_impl->workerCount : 0U;
}

std::size_t JobSystem::defaultWorkerCount() noexcept
{
    const unsigned int count = std::thread::hardware_concurrency();
    return count == 0U ? 1U : static_cast<std::size_t>(count);
}

}  // namespace sprout
