#include "Math.hpp"
#include "TaskScheduler.hpp"
#include "glm/ext/vector_float3.hpp"
#include "gtest/gtest.h"
#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>
#include <vector>
TEST(MemoryAlignTest, StructAlignment)
{
    struct UnalignedStruct
    {
        char a;
        int b;
        double c;
    };
    EXPECT_EQ(alignof(UnalignedStruct), 8); // 默认对齐到8字节
    // a offset
    EXPECT_EQ(offsetof(UnalignedStruct, a), 0);
    // b offset
    EXPECT_EQ(offsetof(UnalignedStruct, b), 4);
    // c offset
    EXPECT_EQ(offsetof(UnalignedStruct, c), 8);
    // total
    EXPECT_EQ(sizeof(UnalignedStruct), 16);
    struct alignas(16) AlignedStruct
    {
        int a;
        float b;
        double c;
    };

    EXPECT_EQ(alignof(AlignedStruct), 16);
    // a offset
    EXPECT_EQ(offsetof(AlignedStruct, a), 0);
    // b offset
    EXPECT_EQ(offsetof(AlignedStruct, b), 4);
    // c offset
    EXPECT_EQ(offsetof(AlignedStruct, c), 8);
    // total
    EXPECT_EQ(sizeof(AlignedStruct), 16);
}

TEST(MemoryAlignTest, StructAlignment2)
{
    struct UnalignedStruct
    {
        bool a;
        bool b;
        bool c;
    };
    // a offset
    EXPECT_EQ(offsetof(UnalignedStruct, a), 0);
    // b offset
    EXPECT_EQ(offsetof(UnalignedStruct, b), 1);
    // c offset
    EXPECT_EQ(offsetof(UnalignedStruct, c), 2);
    // total
    EXPECT_EQ(sizeof(UnalignedStruct), 3);
    struct alignas(16) AlignedStruct
    {
        alignas(16) bool a;
        alignas(16) bool b;
        alignas(16) bool c;
    };
    // a offset
    EXPECT_EQ(offsetof(AlignedStruct, a), 0);
    // b offset
    EXPECT_EQ(offsetof(AlignedStruct, b), 16);
    // c offset
    EXPECT_EQ(offsetof(AlignedStruct, c), 32);
    // total
    EXPECT_EQ(sizeof(AlignedStruct), 48);
}
TEST(MemoryAlignTest, StructAlignment3)
{
    struct UnalignedStruct
    {
        short s;
        char c;
    };
    // s offset
    EXPECT_EQ(offsetof(UnalignedStruct, s), 0);
    // c offset
    EXPECT_EQ(offsetof(UnalignedStruct, c), 2);
    // total
    EXPECT_EQ(sizeof(UnalignedStruct), 4);
    // align
    EXPECT_EQ(alignof(UnalignedStruct), 2);
    struct UnalignedStruct1
    {
        short s;
        double d;
        char c;
    };
    // s offset
    EXPECT_EQ(offsetof(UnalignedStruct1, s), 0);
    // d offset
    EXPECT_EQ(offsetof(UnalignedStruct1, d), 8);
    // c offset
    EXPECT_EQ(offsetof(UnalignedStruct1, c), 16);
    // total
    EXPECT_EQ(sizeof(UnalignedStruct1), 24);
    // align
    EXPECT_EQ(alignof(UnalignedStruct1), 8);
    struct UnalignedStruct2
    {
        int i;
        struct Test
        {
            double d1;
            double d2;
        } t;
    };
    EXPECT_EQ(sizeof(UnalignedStruct2), 24);
}