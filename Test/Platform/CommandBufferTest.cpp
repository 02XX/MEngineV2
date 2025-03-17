#include "CommandBuffeFactory.hpp"
#include "Context.hpp"
#include "gtest/gtest.h"
#include <iostream>
#include <memory>

using namespace MEngine;

class CommandBufferFactoryTest : public ::testing::Test
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
    }
};

TEST_F(CommandBufferFactoryTest, CreatePrimaryCommandBuffer)
{
    auto &context = Context::Instance();
    uint32_t queueFamilyIndex = context.GetQueueFamilyIndicates().graphicsFamily.value();

    CommandBufferFactory factory(queueFamilyIndex);
    vk::UniqueCommandBuffer cmd = factory.CreatePrimaryCommandBuffer();
    EXPECT_TRUE(static_cast<bool>(cmd)) << "Failed to create primary command buffer.";
}

TEST_F(CommandBufferFactoryTest, CreateSecondaryCommandBuffer)
{
    auto &context = Context::Instance();
    uint32_t queueFamilyIndex = context.GetQueueFamilyIndicates().graphicsFamily.value();
    CommandBufferFactory factory(queueFamilyIndex);
    vk::UniqueCommandBuffer cmd = factory.CreateSecondaryCommandBuffer();
    EXPECT_TRUE(static_cast<bool>(cmd)) << "Failed to create secondary command buffer.";
}
TEST_F(CommandBufferFactoryTest, CreateMultiplePrimaryCommandBuffers)
{
    auto &context = Context::Instance();
    uint32_t queueFamilyIndex = context.GetQueueFamilyIndicates().graphicsFamily.value();

    CommandBufferFactory factory(queueFamilyIndex);

    constexpr uint32_t bufferCount = 3;
    auto buffers = factory.CreatePrimaryCommandBuffers(bufferCount);

    EXPECT_EQ(buffers.size(), bufferCount);

    for (auto &buf : buffers)
        EXPECT_TRUE(!!buf);
}

TEST_F(CommandBufferFactoryTest, CreateMultipleSecondaryCommandBuffers)
{
    auto &context = Context::Instance();
    uint32_t queueFamilyIndex = context.GetQueueFamilyIndicates().graphicsFamily.value();

    CommandBufferFactory factory(queueFamilyIndex);

    constexpr uint32_t bufferCount = 3;
    auto buffers = factory.CreateSecondaryCommandBuffers(bufferCount);

    EXPECT_EQ(buffers.size(), bufferCount);
    for (auto &buf : buffers)
        EXPECT_TRUE(!!buf);
}