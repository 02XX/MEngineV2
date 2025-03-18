#pragma once
#include "Context.hpp"
#include "MEngine.hpp"
#include <vector>
#include <vulkan/vulkan.hpp>

namespace MEngine
{
class MENGINE_API SwapchainManager final
{
  private:
    vk::UniqueSwapchainKHR mSwapchain;
    std::vector<vk::UniqueImageView> mSwapchainImageViews;
    void CreateSwapchainImageViews();

  public:
    SwapchainManager(vk::Extent2D extent, vk::SurfaceKHR surface, vk::SwapchainKHR oldSwapchain);
    SwapchainManager(const SwapchainManager &) = delete;
    SwapchainManager &operator=(const SwapchainManager &) = delete;
    SwapchainManager(SwapchainManager &&) = delete;
    SwapchainManager &operator=(SwapchainManager &&) = delete;
    ~SwapchainManager();

    std::vector<vk::ImageView> GetSwapchainImageViews() const;
    std::vector<vk::Image> GetSwapchainImages() const;
    vk::SwapchainKHR GetSwapchain() const;
};
} // namespace MEngine