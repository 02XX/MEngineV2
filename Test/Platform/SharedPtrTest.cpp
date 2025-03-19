#include "gtest/gtest.h"
#include <atomic>
#include <thread>
#include <vector>
using namespace std;

template <typename T> class SharedPtr
{
  private:
    T *mPtr;
    std::atomic<int> *mCounter;

  public:
    SharedPtr() : mPtr(nullptr), mCounter(nullptr)
    {
    }
    SharedPtr(T *ptr) : mPtr(ptr), mCounter(new std::atomic<int>(1))
    {
    }
    SharedPtr(const SharedPtr &other) : mPtr(other.mPtr), mCounter(other.mCounter)
    {
        if (mCounter)
        {
            mCounter->fetch_add(1, std::memory_order_relaxed);
        }
    }
    SharedPtr(SharedPtr &&other) : mPtr(other.mPtr), mCounter(other.mCounter)
    {
        other.mPtr = nullptr;
        other.mCounter = nullptr;
    }
    ~SharedPtr()
    {
        if (mCounter && mCounter->fetch_sub(1, std::memory_order_acq_rel) == 1 && mPtr)
        {
            delete mPtr;
            delete mCounter;
        }
    }
    SharedPtr &operator=(const SharedPtr &other)
    {
        if (this != &other)
        {
            if (mCounter->fetch_sub(1, std::memory_order_acq_rel) == 1 && mPtr)
            {
                delete mPtr;
                delete mCounter;
            }
            mPtr = other.mPtr;
            mCounter = other.mCounter;
            if (mCounter)
            {
                mCounter->fetch_add(1, std::memory_order_relaxed);
            }
        }
        return *this;
    }
    SharedPtr &operator=(SharedPtr &&other)
    {
        if (this != &other)
        {
            if (mCounter && mCounter->fetch_sub(1, std::memory_order_acq_rel) == 1 && mPtr)
            {
                delete mPtr;
                delete mCounter;
            }
            mPtr = other.mPtr;
            mCounter = other.mCounter;
            other.mPtr = nullptr;
            other.mCounter = nullptr;
        }
        return *this;
    }
    void reset()
    {
        if (mCounter->fetch_sub(1, std::memory_order_acq_rel) == 1 && mPtr)
        {
            delete mPtr;
            delete mCounter;
        }
        mPtr = nullptr;
        mCounter = nullptr;
    }
    int useCount() const
    {
        return mCounter ? mCounter->load(std::memory_order_relaxed) : 0;
    }
};

TEST(SharedPtrTest, sharedPtr_Count)
{
    auto ptr = SharedPtr<int>(new int(1));
    EXPECT_EQ(ptr.useCount(), 1);
    auto ptr2 = ptr;
    EXPECT_EQ(ptr.useCount(), 2);
    ptr2.reset();
    EXPECT_EQ(ptr.useCount(), 1);
    auto thread1 = new std::thread([&ptr]() {
        vector<SharedPtr<int>> vec;
        for (int i = 0; i < 100; i++)
        {
            vec.push_back(ptr);
        }
        std::this_thread::sleep_for(chrono::seconds(10));
    });
    auto thread2 = new std::thread([&ptr]() {
        vector<SharedPtr<int>> vec;
        for (int i = 0; i < 100; i++)
        {
            vec.push_back(ptr);
        }
        std::this_thread::sleep_for(chrono::seconds(10));
    });
    this_thread::sleep_for(chrono::seconds(5));
    EXPECT_EQ(ptr.useCount(), 201);
    thread1->join();
    thread2->join();
    EXPECT_EQ(ptr.useCount(), 1);
}