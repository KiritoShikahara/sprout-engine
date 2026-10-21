#include "core/job/JobHandle.hpp"

#include <gtest/gtest.h>

#include <array>
#include <atomic>
#include <chrono>
#include <latch>
#include <stdexcept>
#include <thread>
#include <vector>

namespace {

using sprout::JobHandle;
using sprout::JobSystem;

TEST(JobHandleTests, DefaultHandle_IsInvalid)
{
    const JobHandle handle;

    EXPECT_FALSE(handle.isValid());
    EXPECT_FALSE(static_cast<bool>(handle));
    EXPECT_FALSE(handle.isComplete());
    handle.wait();
}

TEST(JobSystemTests, Submit_RunsJobAndCompletesHandle)
{
    JobSystem jobs(2);
    std::atomic<int> value{0};

    const JobHandle handle = jobs.submit([&value] { value.store(42); });

    EXPECT_TRUE(handle.isValid());
    EXPECT_TRUE(handle);
    jobs.wait(handle);
    EXPECT_TRUE(handle.isComplete());
    EXPECT_EQ(value.load(), 42);
}

TEST(JobSystemTests, Submit_EmptyFunction_ReturnsInvalidHandle)
{
    JobSystem jobs(1);

    const JobHandle handle = jobs.submit(JobSystem::JobFunction{});

    EXPECT_FALSE(handle.isValid());
}

TEST(JobSystemTests, Submit_RunsOffMainThread)
{
    JobSystem jobs(1);
    const std::thread::id mainThread = std::this_thread::get_id();
    std::atomic<bool> ranOnMainThread{true};

    const JobHandle handle = jobs.submit([&ranOnMainThread, &mainThread] {
        ranOnMainThread.store(std::this_thread::get_id() == mainThread);
    });
    jobs.wait(handle);

    EXPECT_FALSE(ranOnMainThread.load());
}

TEST(JobSystemTests, Dependency_RunsAfterPrerequisite)
{
    JobSystem jobs(2);
    std::atomic<int> order{0};
    std::atomic<int> firstOrder{0};
    std::atomic<bool> dependencyHeld{false};

    const JobHandle first = jobs.submit([&order, &firstOrder] {
        firstOrder.store(order.fetch_add(1) + 1);
    });

    const std::array<JobHandle, 1> dependencies{first};
    const JobHandle second = jobs.submit(
        [&order, &firstOrder, &dependencyHeld] {
            dependencyHeld.store(firstOrder.load() == 1 && order.load() == 1);
            order.fetch_add(1);
        },
        dependencies);

    jobs.wait(second);

    EXPECT_TRUE(first.isComplete());
    EXPECT_TRUE(second.isComplete());
    EXPECT_TRUE(dependencyHeld.load());
    EXPECT_EQ(order.load(), 2);
}

TEST(JobSystemTests, Dependencies_WaitForAllPrerequisites)
{
    JobSystem jobs(2);
    std::latch gate{1};
    std::atomic<bool> firstDone{false};
    std::atomic<bool> secondDone{false};
    std::atomic<bool> ran{false};
    std::atomic<bool> sawBoth{false};

    const JobHandle first = jobs.submit([&gate, &firstDone] {
        gate.wait();
        firstDone.store(true);
    });
    const JobHandle second = jobs.submit([&gate, &secondDone] {
        gate.wait();
        secondDone.store(true);
    });

    const std::array<JobHandle, 2> dependencies{first, second};
    const JobHandle third = jobs.submit(
        [&firstDone, &secondDone, &ran, &sawBoth] {
            sawBoth.store(firstDone.load() && secondDone.load());
            ran.store(true);
        },
        dependencies);

    EXPECT_FALSE(ran.load());
    gate.count_down();
    jobs.wait(third);

    EXPECT_TRUE(ran.load());
    EXPECT_TRUE(sawBoth.load());
}

TEST(JobSystemTests, Dependency_AlreadyComplete_Runs)
{
    JobSystem jobs(1);

    const JobHandle first = jobs.submit([] {});
    jobs.wait(first);

    std::atomic<bool> ran{false};
    const std::array<JobHandle, 1> dependencies{first};
    const JobHandle second = jobs.submit([&ran] { ran.store(true); }, dependencies);
    jobs.wait(second);

    EXPECT_TRUE(ran.load());
}

TEST(JobSystemTests, MainThreadContinuation_RunsOnPump)
{
    JobSystem jobs(2);
    const std::thread::id mainThread = std::this_thread::get_id();
    std::atomic<int> runs{0};
    std::thread::id callbackThread;

    const JobHandle handle = jobs.submit([&jobs, &runs, &callbackThread] {
        jobs.postMainThread([&runs, &callbackThread] {
            callbackThread = std::this_thread::get_id();
            runs.fetch_add(1);
        });
    });

    jobs.wait(handle);
    EXPECT_EQ(runs.load(), 0);
    EXPECT_EQ(jobs.pumpMainThread(), 1U);
    EXPECT_EQ(runs.load(), 1);
    EXPECT_EQ(callbackThread, mainThread);
    EXPECT_EQ(jobs.pumpMainThread(), 0U);
}

TEST(JobSystemTests, MainThreadContinuations_RunInSubmissionOrder)
{
    JobSystem jobs(1);
    std::vector<int> order;

    jobs.postMainThread([&order] { order.push_back(1); });
    jobs.postMainThread([&order] { order.push_back(2); });

    EXPECT_EQ(jobs.pumpMainThread(), 2U);
    EXPECT_EQ(order, (std::vector<int>{1, 2}));
}

TEST(JobSystemTests, WaitAll_WaitsForEverySubmittedJob)
{
    JobSystem jobs(4);
    std::atomic<int> completed{0};
    constexpr int kJobCount = 64;

    for (int index = 0; index < kJobCount; ++index) {
        (void)jobs.submit([&completed] { completed.fetch_add(1); });
    }

    jobs.waitAll();

    EXPECT_EQ(completed.load(), kJobCount);
}

TEST(JobSystemTests, ThrowingJob_DoesNotEscapeAndCompletes)
{
    JobSystem jobs(1);

    const JobHandle handle = jobs.submit([] { throw std::runtime_error{"boom"}; });
    jobs.wait(handle);

    EXPECT_TRUE(handle.isComplete());
}

TEST(JobSystemTests, Stop_CancelsQueuedJobsAndRejectsNewWork)
{
    JobSystem jobs(1);
    std::latch started{1};
    std::latch release{1};
    std::atomic<bool> queuedRan{false};

    const JobHandle running = jobs.submit([&started, &release] {
        started.count_down();
        release.wait();
    });
    const JobHandle queued = jobs.submit([&queuedRan] { queuedRan.store(true); });

    started.wait();

    std::thread stopper{[&jobs] { jobs.stop(); }};
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (jobs.isRunning() && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::yield();
    }
    release.count_down();
    stopper.join();

    EXPECT_FALSE(queuedRan.load());
    EXPECT_TRUE(running.isComplete());
    EXPECT_TRUE(queued.isComplete());
    EXPECT_FALSE(jobs.isRunning());
    EXPECT_FALSE(jobs.submit([] {}).isValid());

    jobs.stop();
}

TEST(JobSystemTests, Stop_ClearsMainThreadContinuations)
{
    JobSystem jobs(1);
    std::atomic<bool> ran{false};

    jobs.postMainThread([&ran] { ran.store(true); });
    jobs.stop();

    EXPECT_EQ(jobs.pumpMainThread(), 0U);
    EXPECT_FALSE(ran.load());

    jobs.postMainThread([&ran] { ran.store(true); });
    EXPECT_EQ(jobs.pumpMainThread(), 0U);
    EXPECT_FALSE(ran.load());
}

TEST(JobSystemTests, DefaultWorkerCount_IsPositive)
{
    EXPECT_GE(JobSystem::defaultWorkerCount(), 1U);

    JobSystem jobs;
    EXPECT_TRUE(jobs.isRunning());
    EXPECT_GE(jobs.workerCount(), 1U);
}

}  // namespace
