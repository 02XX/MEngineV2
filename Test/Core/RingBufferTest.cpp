#include "RingBuffer.hpp"
#include "gtest/gtest.h"
#include <stdexcept>
#include <vector>

using namespace MEngine;

TEST(RingBufferTest, Constructor)
{
    // Test valid capacity
    RingBuffer<int> buffer(5);
    EXPECT_EQ(buffer.Capacity(), 5);
    EXPECT_EQ(buffer.Size(), 0);
    EXPECT_TRUE(buffer.IsEmpty());
    EXPECT_FALSE(buffer.IsFull());

    // Test zero capacity
    EXPECT_THROW(RingBuffer<int>(0), std::invalid_argument);
}

TEST(RingBufferTest, PushAndPop)
{
    RingBuffer<int> buffer(3);

    // Push elements
    buffer.Push(1);
    EXPECT_EQ(buffer.Size(), 1);
    buffer.Push(2);
    EXPECT_EQ(buffer.Size(), 2);
    buffer.Push(3);
    EXPECT_EQ(buffer.Size(), 3);
    EXPECT_TRUE(buffer.IsFull());

    // Test overflow
    EXPECT_THROW(buffer.Push(4), std::runtime_error);

    // Pop elements
    EXPECT_EQ(buffer.Pop(), 1);
    EXPECT_EQ(buffer.Size(), 2);
    EXPECT_EQ(buffer.Pop(), 2);
    EXPECT_EQ(buffer.Size(), 1);
    EXPECT_EQ(buffer.Pop(), 3);
    EXPECT_EQ(buffer.Size(), 0);
    EXPECT_TRUE(buffer.IsEmpty());

    // Test underflow
    EXPECT_THROW(buffer.Pop(), std::runtime_error);
}

TEST(RingBufferTest, PushOverwrite)
{
    RingBuffer<int> buffer(3);

    // Fill the buffer
    buffer.PushOverwrite(1);
    buffer.PushOverwrite(2);
    buffer.PushOverwrite(3);
    EXPECT_TRUE(buffer.IsFull());

    // Overwrite oldest element
    buffer.PushOverwrite(4);
    EXPECT_EQ(buffer.Size(), 3);
    EXPECT_EQ(buffer.Pop(), 2); // 1 should be overwritten
    EXPECT_EQ(buffer.Pop(), 3);
    EXPECT_EQ(buffer.Pop(), 4);
}

TEST(RingBufferTest, CircularBehavior)
{
    RingBuffer<int> buffer(3);

    // Fill and partially empty
    buffer.Push(1);
    buffer.Push(2);
    buffer.Push(3);
    buffer.Pop();
    buffer.Pop();

    // Add more elements to test circular behavior
    buffer.Push(4);
    buffer.Push(5);

    EXPECT_EQ(buffer.Size(), 3);
    EXPECT_EQ(buffer.Pop(), 3);
    EXPECT_EQ(buffer.Pop(), 4);
    EXPECT_EQ(buffer.Pop(), 5);
}

TEST(RingBufferTest, Iterator)
{
    RingBuffer<int> buffer(5);

    // Test empty buffer iterator
    EXPECT_EQ(buffer.begin(), buffer.end());

    // Add elements
    buffer.Push(1);
    buffer.Push(2);
    buffer.Push(3);

    // Test iterator
    std::vector<int> result;
    for (auto item : buffer)
    {
        result.push_back(item);
    }
    EXPECT_EQ(result, std::vector<int>({1, 2, 3}));

    // Test iterator after pop
    buffer.Pop();
    result.clear();
    for (auto item : buffer)
    {
        result.push_back(item);
    }
    EXPECT_EQ(result, std::vector<int>({2, 3}));

    // Test iterator with circular behavior
    buffer.Push(4);
    buffer.Push(5);
    buffer.Push(6); // This will throw because capacity is 5 and we've only popped once
}
TEST(RingBufferTest, ForIterator)
{
    RingBuffer<int> buffer(5);

    // Fill the buffer
    buffer.Push(1);
    buffer.Push(2);
    buffer.Push(3);
    buffer.Push(4);
    buffer.Push(5);
    EXPECT_TRUE(buffer.IsFull());
    for (auto item : buffer)
    {
        std::cout << item << " ";
    }
    std::cout << std::endl;
    buffer.PushOverwrite(6);
    for (auto item : buffer)
    {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}

TEST(RingBufferTest, ComplexType)
{
    RingBuffer<std::string> buffer(2);

    buffer.Push("hello");
    buffer.Push("world");

    EXPECT_EQ(buffer.Pop(), "hello");
    EXPECT_EQ(buffer.Pop(), "world");

    buffer.PushOverwrite("test");
    buffer.PushOverwrite("overwrite");

    EXPECT_EQ(buffer.Pop(), "test");
    EXPECT_EQ(buffer.Pop(), "overwrite");
}

TEST(RingBufferTest, CopyAndMoveSemantics)
{
    RingBuffer<int> buffer(3);
    buffer.Push(1);
    buffer.Push(2);

    // Test copy constructor (should be deleted)
    // RingBuffer<int> copy(buffer); // Should not compile

    // Test move constructor (should be deleted)
    // RingBuffer<int> moved(std::move(buffer)); // Should not compile
}

TEST(RingBufferTest, EdgeCases)
{
    // Single element buffer
    RingBuffer<int> buffer(1);

    buffer.Push(42);
    EXPECT_TRUE(buffer.IsFull());
    EXPECT_THROW(buffer.Push(43), std::runtime_error);

    EXPECT_EQ(buffer.Pop(), 42);
    EXPECT_TRUE(buffer.IsEmpty());

    buffer.PushOverwrite(44);
    buffer.PushOverwrite(45); // Should overwrite
    EXPECT_EQ(buffer.Pop(), 45);
}