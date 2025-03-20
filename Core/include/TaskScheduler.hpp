#pragma once
#include "Logger.hpp"
#include "MEngine.hpp"
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <queue>
#include <thread>
#include <vector>
namespace MEngine
{
class MENGINE_API TaskScheduler final
{
  private:
    int64_t mThreadCount;
    int64_t mTaskCount;
    std::vector<std::thread> mWorkers;
    std::queue<std::function<void()>> mTasks;
    std::atomic<bool> mStop;
    std::condition_variable mNotFull;
    std::condition_variable mNotEmpty;
    std::atomic<int64_t> mPendingTasks{0};
    std::mutex mMutex, mWaitMutex;
    std::condition_variable mAllTasksDone;

  public:
    TaskScheduler(int64_t threadCount, int64_t taskCount);
    TaskScheduler(const TaskScheduler &) = delete;
    TaskScheduler &operator=(const TaskScheduler &) = delete;
    TaskScheduler(TaskScheduler &&) = delete;
    TaskScheduler &operator=(TaskScheduler &&) = delete;
    ~TaskScheduler();
    void WaitAll();
    void AddTask(std::function<void()> &&task);
};
} // namespace MEngine