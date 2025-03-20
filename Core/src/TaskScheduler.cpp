#include "TaskScheduler.hpp"
#include <mutex>

namespace MEngine
{
TaskScheduler::TaskScheduler(int64_t threadCount, int64_t taskCount)
    : mStop(false), mThreadCount(threadCount), mTaskCount(taskCount)
{
    for (int64_t i = 0; i < threadCount; i++)
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
                task();
                mPendingTasks--;
                if (mPendingTasks == 0)
                {
                    std::unique_lock<std::mutex> lock(mWaitMutex);
                    mAllTasksDone.notify_all();
                }
                mNotFull.notify_one();
            }
            LogT("TaskScheduler's worker {} thread exit", std::this_thread::get_id());
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
} // namespace MEngine