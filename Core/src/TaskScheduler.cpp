#include "TaskScheduler.hpp"

namespace MEngine
{
Task::Task(std::function<void()> &&task) : mTask(task)
{
}

void Task::Execute()
{
    try
    {
        mTask();
        mDone = true;
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mCondition.notify_all();
        }
    }
    catch (const std::exception &e)
    {
        mDone = true;
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mCondition.notify_all();
        }
    }
}

bool Task::IsDone() const noexcept
{
    return mDone.load();
}

void Task::Wait()
{
    std::unique_lock<std::mutex> lock(mMutex);
    mCondition.wait(lock, [this]() { return mDone.load(); });
}

std::shared_ptr<Task> Task::Run(std::function<void()> &&task)
{
    std::shared_ptr<Task> t = std::make_shared<Task>(std::move(task));
    TaskScheduler::Instance().AddTask(t);
    return t;
}

void Task::WhenAll(std::vector<std::shared_ptr<Task>> tasks)
{
    for (auto &task : tasks)
    {
        task->Wait();
    }
}

// void Task::WhenAny(std::vector<std::shared_ptr<Task>> tasks)
// {

// }

TaskScheduler &TaskScheduler::Instance()
{
    static TaskScheduler instance;
    return instance;
}
void TaskScheduler::Initialize(uint32_t threadCount, uint32_t taskCount)
{
    mTaskCount = taskCount;
    mThreadCount = threadCount;
    mStop = false;
    for (int i = 0; i < threadCount; i++)
    {
        mWorkers.emplace_back([this]() {
            while (true)
            {
                std::shared_ptr<Task> task;
                {
                    std::unique_lock<std::mutex> lock(mMutex);
                    mNotEmpty.wait(lock, [this]() { return mStop || !mTasks.empty(); });
                    if (mStop && mTasks.empty())
                    {
                        break;
                    }
                    task = mTasks.front();
                    mTasks.pop();
                }
                task->Execute();
                mPendingTasks--;
                mNotFull.notify_one();
            }
            std::stringstream ss;
            ss << std::this_thread::get_id();
            std::string threadID = ss.str();
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
void TaskScheduler::AddTask(std::shared_ptr<Task> task)
{
    {
        std::unique_lock<std::mutex> lock(mMutex);
        mNotFull.wait(lock, [this]() { return mTasks.size() < mTaskCount; });
        mTasks.push(task);
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