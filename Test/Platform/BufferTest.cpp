#include "BufferFactory.hpp"
#include "Context.hpp"
#include "gtest/gtest.h"
#include <cstring>

using namespace MEngine;

class BufferFactoryTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        auto &context = Context::Instance();

        const std::vector<const char *> deviceLayers = {"VK_LAYER_KHRONOS_validation"};
        const std::vector<const char *> deviceExtensions = {"VK_KHR_swapchain"};

        context.Init(
            [](vk::Instance instance) -> vk::SurfaceKHR {
                VkSurfaceKHR surface;
                VkHeadlessSurfaceCreateInfoEXT createInfo{};
                createInfo.sType = VK_STRUCTURE_TYPE_HEADLESS_SURFACE_CREATE_INFO_EXT;
                vkCreateHeadlessSurfaceEXT(instance, &createInfo, nullptr, &surface);
                return surface;
            },
            {}, deviceExtensions, deviceLayers);
        mFactory = std::make_unique<BufferFactory>();
    }
    std::unique_ptr<BufferFactory> mFactory;
};

// 顶点缓冲区基础功能测试
TEST_F(BufferFactoryTest, CreateVertexBufferBasic)
{
    const float vertexData[] = {0.0f, 1.0f, 2.0f};
    const auto bufferSize = sizeof(vertexData);

    auto buffer = mFactory->CreateVertexBuffer(bufferSize, vertexData);
    ASSERT_TRUE(buffer) << "Vertex buffer creation failed";
}

// 索引缓冲区类型验证
TEST_F(BufferFactoryTest, CreateIndexBufferType)
{
    const uint32_t indexData[] = {0, 1, 2, 3};
    auto buffer = mFactory->CreateIndexBuffer(sizeof(indexData), indexData);

    ASSERT_TRUE(buffer) << "Index buffer creation failed";
}

// Uniform缓冲区内存特性测试
TEST_F(BufferFactoryTest, UniformBufferMemoryProperties)
{
    const vk::DeviceSize bufferSize = 256;
    auto buffer = mFactory->CreateUniformBuffer(bufferSize);

    ASSERT_TRUE(buffer) << "Uniform buffer creation failed";

    const auto &allocInfo = buffer->GetAllocationInfo();
    EXPECT_GE(allocInfo.size, bufferSize) << "Memory allocation size insufficient";
    EXPECT_TRUE(allocInfo.pMappedData) << "Uniform buffer should be host-mapped";
}

// 暂存缓冲区拷贝功能验证
TEST_F(BufferFactoryTest, StagingBufferCopyFunctionality)
{
    // 准备测试数据
    const char testData[] = "Test staging buffer data";
    const auto dataSize = sizeof(testData);

    // 创建源缓冲区
    auto stagingBuffer = mFactory->CreateStagingBuffer(dataSize, testData);
    ASSERT_TRUE(stagingBuffer) << "Staging buffer creation failed";

    // 创建目标顶点缓冲区
    auto vertexBuffer = mFactory->CreateVertexBuffer(dataSize);
    ASSERT_TRUE(vertexBuffer) << "Vertex buffer creation failed";

    // 执行缓冲区拷贝
    mFactory->CopyBuffer(stagingBuffer->GetBuffer(), vertexBuffer->GetBuffer(), dataSize);
}

// 异常情况测试：创建0大小缓冲区
TEST_F(BufferFactoryTest, CreateZeroSizeBuffer)
{
    EXPECT_THROW({ mFactory->CreateVertexBuffer(0); }, std::runtime_error) << "Should reject zero-size buffer";
}

class BufferMoveSemanticsTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        auto &context = Context::Instance();

        const std::vector<const char *> deviceLayers = {"VK_LAYER_KHRONOS_validation"};
        const std::vector<const char *> deviceExtensions = {"VK_KHR_swapchain"};

        context.Init(
            [](vk::Instance instance) -> vk::SurfaceKHR {
                VkSurfaceKHR surface;
                VkHeadlessSurfaceCreateInfoEXT createInfo{};
                createInfo.sType = VK_STRUCTURE_TYPE_HEADLESS_SURFACE_CREATE_INFO_EXT;
                vkCreateHeadlessSurfaceEXT(instance, &createInfo, nullptr, &surface);
                return surface;
            },
            {}, deviceExtensions, deviceLayers);

        vk::DeviceSize size = 1024;
        vk::BufferUsageFlags usage = vk::BufferUsageFlagBits::eVertexBuffer;
        mValidBuffer = std::make_unique<Buffer>(size, usage, VMA_MEMORY_USAGE_GPU_ONLY);
        ASSERT_TRUE(mValidBuffer->GetBuffer()) << "Test buffer creation failed";
    }
    std::unique_ptr<Buffer> mValidBuffer;
};
