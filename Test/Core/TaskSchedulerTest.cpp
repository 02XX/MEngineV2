#include "TaskScheduler.hpp"
#include "gtest/gtest.h"
#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>
#include <vector>

using namespace std::chrono_literals;
using namespace MEngine;
// 测试构造函数是否正确初始化
TEST(TaskSchedulerTest, ConstructorInitialization)
{
    // 给定参数
    const int64_t threadCount = 4;
    const int64_t maxTaskCount = 10;

    // 构造调度器
    auto &scheduler = TaskScheduler::Instance();
    TaskScheduler::Instance().Initialize(threadCount, maxTaskCount);

    EXPECT_EQ(scheduler.GetThreadCount(), threadCount);
    EXPECT_EQ(scheduler.GetTaskCount(), maxTaskCount);
    EXPECT_EQ(scheduler.GetPendingTasks(), 0);
}

// 测试添加任务是否成功（单任务）
TEST(TaskSchedulerTest, AddSingleTask)
{

    auto &scheduler = TaskScheduler::Instance();
    TaskScheduler::Instance().Initialize(2, 5);
    std::atomic<int> counter{0};

    // 添加一个任务
    auto task = std::make_shared<Task>([&counter]() { counter++; });
    scheduler.AddTask(task);
    Task::WhenAll({task});

    EXPECT_EQ(counter.load(), 1);

    EXPECT_EQ(scheduler.GetPendingTasks(), 0);
}

// 测试多任务并发执行
TEST(TaskSchedulerTest, MultipleTasksExecution)
{
    auto &scheduler = TaskScheduler::Instance();
    TaskScheduler::Instance().Initialize(4, 100);
    std::atomic<int> counter{0};
    const int totalTasks = 1000;

    // 添加1000个任务
    std::vector<std::shared_ptr<Task>> tasks;
    for (int i = 0; i < totalTasks; ++i)
    {
        auto task = std::make_shared<Task>([&counter]() { counter++; });
        scheduler.AddTask(task);
        tasks.push_back(task);
    }

    Task::WhenAll(tasks);
    EXPECT_EQ(counter.load(), totalTasks);
    EXPECT_EQ(scheduler.GetPendingTasks(), 0);
}

// 测试任务队列满时的阻塞行为
TEST(TaskSchedulerTest, BlockWhenTaskQueueFull)
{
    const int64_t maxTasks = 5;
    auto &scheduler = TaskScheduler::Instance();
    TaskScheduler::Instance().Initialize(1, 5);

    std::atomic<int> counter{0};
    std::vector<std::shared_ptr<Task>> tasks;
    std::thread producer([&]() {
        // 尝试添加6个任务（超过队列容量）
        for (int i = 0; i < maxTasks + 1; ++i)
        {
            auto task = std::make_shared<Task>([&counter]() {
                std::this_thread::sleep_for(100ms); // 模拟耗时任务
                counter++;
            });
            scheduler.AddTask(task);
            tasks.push_back(task);
        }
    });
    // 等待队列满后，AddTask 应该阻塞直到队列有空位
    std::this_thread::sleep_for(50ms);
    EXPECT_LE(counter.load(), maxTasks); // 此时最多完成5个任务

    producer.join();
    Task::WhenAll(tasks);
    EXPECT_EQ(counter.load(), maxTasks + 1); // 最终所有任务完成
    EXPECT_EQ(scheduler.GetPendingTasks(), 0);
}

// 测试WaitAll是否等待所有任务完成
TEST(TaskSchedulerTest, WaitAllCompletesAllTasks)
{
    auto &scheduler = TaskScheduler::Instance();
    TaskScheduler::Instance().Initialize(2, 10);
    std::atomic<int> counter{0};
    const int slowTasks = 5;

    std::vector<std::shared_ptr<Task>> tasks;
    // 添加5个耗时任务
    for (int i = 0; i < slowTasks; ++i)
    {
        auto task = std::make_shared<Task>([&counter]() {
            std::this_thread::sleep_for(100ms);
            counter++;
        });
        scheduler.AddTask(task);
        tasks.push_back(task);
    }

    // 立即调用WaitAll，应等待所有任务完成
    Task::WhenAll(tasks);
    EXPECT_EQ(counter.load(), slowTasks);
    EXPECT_EQ(scheduler.GetPendingTasks(), 0);
}

// 测试高并发场景下的线程安全
TEST(TaskSchedulerTest, HighConcurrency)
{
    auto &scheduler = TaskScheduler::Instance();
    TaskScheduler::Instance().Initialize(8, 10);
    std::atomic<int> counter{0};
    const int totalTasks = 1000;
    std::vector<std::shared_ptr<Task>> tasks;
    std::vector<std::thread> producers;
    std::mutex mutex;
    // 启动4个生产者线程，每个生产250个任务
    for (int i = 0; i < 4; ++i)
    {
        producers.emplace_back([&]() {
            for (int j = 0; j < totalTasks / 4; ++j)
            {
                auto task = std::make_shared<Task>([&counter]() { counter++; });
                scheduler.AddTask(task);
                std::lock_guard<std::mutex> lock(mutex);
                tasks.push_back(task);
            }
        });
    }

    // 等待所有生产者完成
    for (auto &t : producers)
        t.join();

    // 等待所有任务完成
    Task::WhenAll(tasks);
    EXPECT_EQ(counter.load(), totalTasks);
}

// 测试异常任务不影响调度器
TEST(TaskSchedulerTest, ExceptionHandling)
{
    auto &scheduler = TaskScheduler::Instance();
    TaskScheduler::Instance().Initialize(2, 10);
    std::atomic<int> counter{0};

    std::vector<std::shared_ptr<Task>> tasks;
    // 添加一个抛出异常的任务
    auto task1 = std::make_shared<Task>([&counter]() {
        counter++;
        throw std::runtime_error("Oops!");
    });
    scheduler.AddTask(task1);

    // 等待所有任务完成（即使有异常）
    EXPECT_NO_THROW(Task::WhenAll({task1}));
    EXPECT_EQ(counter.load(), 1); // 两个任务都应执行完毕
}

TEST(Task, Run)
{
    std::atomic<int> counter{0};
    auto task = Task::Run([&counter]() { counter++; });
    task->Wait();
    EXPECT_EQ(counter.load(), 1);
}
