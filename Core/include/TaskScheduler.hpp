#pragma once
#include "Logger.hpp"
#include "MEngine.hpp"
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>
namespace MEngine
{
class MENGINE_API Task
{
    friend class TaskScheduler;

  private:
    std::atomic<bool> mDone{false};
    std::function<void()> mTask;
    std::mutex mMutex;
    std::condition_variable mCondition;

  public:
    Task(std::function<void()> &&task);
    Task(const Task &) = delete;
    Task &operator=(const Task &) = delete;
    Task(Task &&other) = delete;
    Task &operator=(Task &&other) = delete;
    ~Task() = default;

    void Execute();
    bool IsDone() const noexcept;
    void Wait();

    static std::shared_ptr<Task> Run(std::function<void()> &&task);
    static void WhenAll(std::vector<std::shared_ptr<Task>> tasks);
    // static void WhenAny(std::vector<std::shared_ptr<Task>> tasks);
};

class MENGINE_API TaskScheduler final
{
  private:
    uint32_t mThreadCount;
    uint32_t mTaskCount;
    std::vector<std::thread> mWorkers;
    std::queue<std::shared_ptr<Task>> mTasks;
    std::atomic<bool> mStop;
    std::condition_variable mNotFull;
    std::condition_variable mNotEmpty;
    std::atomic<uint32_t> mPendingTasks{0};
    std::mutex mMutex;
    TaskScheduler() = default;
    TaskScheduler(const TaskScheduler &) = delete;
    TaskScheduler &operator=(const TaskScheduler &) = delete;
    TaskScheduler(TaskScheduler &&) = delete;
    TaskScheduler &operator=(TaskScheduler &&) = delete;

  public:
    ~TaskScheduler();
    static TaskScheduler &Instance();
    void Initialize(uint32_t threadCount, uint32_t taskCount);
    uint32_t GetThreadCount() const noexcept;
    uint32_t GetTaskCount() const noexcept;
    uint32_t GetPendingTasks() const noexcept;
    void AddTask(std::shared_ptr<Task> task);
};
} // namespace MEngine