
#include "Context.hpp"
#include "Logger.hpp"
#include "PipelineLayoutManager.hpp"
#include "SDL3/SDL.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_video.h"
#include "SDL3/SDL_vulkan.h"
#include "gtest/gtest.h"
#include <cstddef>
#include <iostream>
using namespace MEngine;
using namespace std;

class PipelineLayoutManagerTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        // sdl
        if (!SDL_Init(SDL_INIT_VIDEO))
        {
            LogE("Failed to initialize SDL: {}", SDL_GetError());
        }
        mWindow = SDL_CreateWindow("MEngine", 800, 600, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
        if (!mWindow)
        {
            LogE("Failed to create window: {}", SDL_GetError());
            throw std::runtime_error("Failed to create window");
        }
        Uint32 extensionCount = 0;
        auto extensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);
        std::vector<const char *> requiredExtensions(extensions, extensions + extensionCount);
        // vulkan
        auto &context = Context::Instance();
        context.Init(
            [&](vk::Instance instance) -> vk::SurfaceKHR {
                VkSurfaceKHR surface;
                if (!SDL_Vulkan_CreateSurface(mWindow, static_cast<VkInstance>(instance), nullptr, &surface))
                {
                    LogE("Failed to create Vulkan surface: {}", SDL_GetError());
                    throw std::runtime_error("Failed to create Vulkan surface");
                }
                return vk::SurfaceKHR(surface);
            },
            requiredExtensions, {});
        mManager = std::make_unique<PipelineLayoutManager>();
    }
    std::unique_ptr<PipelineLayoutManager> mManager;
    SDL_Window *mWindow;
};

TEST_F(PipelineLayoutManagerTest, sharedPipelineLayout_destroy)
{
    Logger::Instance().SetLevel(LogLevel::TRACE);
    std::vector<DescriptorBindingInfo> descriptorBindings;
    descriptorBindings.push_back({0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex});
    vk::PushConstantRange pushConstantRange;
    pushConstantRange.setStageFlags(vk::ShaderStageFlagBits::eVertex).setSize(4).setOffset(0);
    {
        auto pipelineLayout = mManager->CreateSharedPipelineLayout(descriptorBindings, {pushConstantRange});
    }
}
