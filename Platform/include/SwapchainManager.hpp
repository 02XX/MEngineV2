#pragma once
#include "Context.hpp"
#include "MEngine.hpp"
#include <vector>
#include <vulkan/vulkan.hpp>

namespace MEngine
{
class MENGINE_API SwapchainManager final
{
  public:
    SwapchainManager();
    vk::UniqueSwapchainKHR CreateSwapchain(vk::Extent2D extent, vk::SurfaceKHR surface, vk::SwapchainKHR oldSwapchain);
    std::vector<vk::UniqueImageView> CreateSwapchainImageViews(vk::SwapchainKHR swapchain);
    std::vector<vk::Image> GetSwapchainImages(vk::SwapchainKHR swapchain);
};
} // namespace MEngine