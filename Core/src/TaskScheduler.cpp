#include "TaskScheduler.hpp"
#include "Logger.hpp"

namespace MEngine
{
TaskScheduler::TaskScheduler(uint32_t threadCount, uint32_t taskCount)
    : mStop(false), mThreadCount(threadCount), mTaskCount(taskCount)
{
    for (int i = 0; i < threadCount; i++)
    {
        mWorkers.emplace_back([this]() {
            while (true)
            {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(mMutex);
                    mNotEmpty.wait(lock, [this]() { return mStop || !mTasks.empty(); });
                    if (mStop && mTasks.empty())
                    {
                        break;
                    }
                    task = std::move(mTasks.front());
                    mTasks.pop();
                }
                try
                {
                    task();
                }
                catch (const std::exception &e)
                {
                    LogE("Task failed: {}", e.what());
                }
                mPendingTasks--;
                if (mPendingTasks == 0)
                {
                    std::unique_lock<std::mutex> lock(mWaitMutex);
                    mAllTasksDone.notify_all();
                }
                mNotFull.notify_one();
            }
            std::stringstream ss;
            ss << std::this_thread::get_id();
            std::string threadID = ss.str();
            LogT("TaskScheduler's worker {} thread exit", threadID);
        });
    }
}

TaskScheduler::~TaskScheduler()
{
    {
        std::unique_lock<std::mutex> lock(mMutex);
        mStop = true;
    }
    mNotEmpty.notify_all();
    for (auto &worker : mWorkers)
    {
        worker.join();
    }
}
void TaskScheduler::WaitAll()
{
    std::unique_lock<std::mutex> lock(mWaitMutex);
    mAllTasksDone.wait(lock, [this]() { return mPendingTasks.load() == 0; });
}
void TaskScheduler::AddTask(std::function<void()> &&task)
{
    {
        std::unique_lock<std::mutex> lock(mMutex);
        mNotFull.wait(lock, [this]() { return mTasks.size() < mTaskCount; });
        mTasks.push(std::move(task));
        mPendingTasks++;
    }
    mNotEmpty.notify_one();
}

uint32_t TaskScheduler::GetThreadCount() const noexcept
{
    return mThreadCount;
}
uint32_t TaskScheduler::GetTaskCount() const noexcept
{
    return mTaskCount;
}
uint32_t TaskScheduler::GetPendingTasks() const noexcept
{
    return mPendingTasks.load();
}
} // namespace MEngine