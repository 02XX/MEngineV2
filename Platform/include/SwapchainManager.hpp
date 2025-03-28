#pragma once
#include "Context.hpp"
#include "MEngine.hpp"
#include "NoCopyable.hpp"
#include <vector>
#include <vulkan/vulkan.hpp>

namespace MEngine
{
class MENGINE_API SwapchainManager final : public NoCopyable
{
  private:
    vk::UniqueSwapchainKHR mSwapchain;
    std::vector<vk::UniqueImageView> mSwapchainImageViews;
    void CreateSwapchainImageViews();

  public:
    SwapchainManager(vk::Extent2D extent, vk::SurfaceKHR surface, vk::SwapchainKHR oldSwapchain);

    std::vector<vk::ImageView> GetSwapchainImageViews() const;
    std::vector<vk::Image> GetSwapchainImages() const;
    vk::SwapchainKHR GetSwapchain() const;
};
} // namespace MEngine