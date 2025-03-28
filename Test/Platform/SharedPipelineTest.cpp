
#include "Context.hpp"
#include "Logger.hpp"
#include "PipelineManager.hpp"
#include "gtest/gtest.h"
#include <iostream>
#include <vulkan/vulkan.hpp>
using namespace MEngine;
using namespace std;

class PipelineManagerTest : public ::testing::Test
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
        mManager = std::make_unique<PipelineManager>();
    }
    std::unique_ptr<PipelineManager> mManager;
};

TEST_F(PipelineManagerTest, sharedPipeline_destroy)
{
    Logger::Instance().SetLevel(LogLevel::LOG_TRACE);
    {
        auto pipeline = mManager->CreateUniqueGraphicsPipeline({});
    }
}