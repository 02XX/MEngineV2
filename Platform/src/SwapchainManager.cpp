#include "SwapchainManager.hpp"
#include <vector>

namespace MEngine
{
SwapchainManager::SwapchainManager(vk::Extent2D extent, vk::SurfaceKHR surface, vk::SwapchainKHR oldSwapchain)
{
    auto &context = Context::Instance();
    auto &surfaceInfo = context.GetSurfaceInfo();
    auto &queueFamilyIndices = context.GetQueueFamilyIndicates();
    vk::SwapchainCreateInfoKHR swapchainCreateInfo;

    swapchainCreateInfo.setSurface(surface)
        .setClipped(true)
        .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
        .setImageArrayLayers(surfaceInfo.imageArrayLayer)
        .setImageColorSpace(surfaceInfo.format.colorSpace)
        .setImageExtent(surfaceInfo.extent)
        .setImageFormat(surfaceInfo.format.format)
        .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
        .setPresentMode(surfaceInfo.presentMode)
        .setPreTransform(vk::SurfaceTransformFlagBitsKHR::eIdentity)
        .setMinImageCount(surfaceInfo.imageCount)
        .setOldSwapchain(oldSwapchain);
    if (queueFamilyIndices.graphicsFamily == queueFamilyIndices.presentFamily)
    {
        swapchainCreateInfo.setImageSharingMode(vk::SharingMode::eExclusive)
            .setQueueFamilyIndices({queueFamilyIndices.graphicsFamily.value()});
    }
    else
    {
        std::array<uint32_t, 2> queueFamilyIndicesArray = {queueFamilyIndices.graphicsFamily.value(),
                                                           queueFamilyIndices.presentFamily.value()};
        swapchainCreateInfo.setImageSharingMode(vk::SharingMode::eConcurrent)
            .setQueueFamilyIndices(queueFamilyIndicesArray);
    }
    mSwapchain = context.GetDevice().createSwapchainKHRUnique(swapchainCreateInfo);
    LogD("Swapchain Created.");
    CreateSwapchainImageViews();
}

void SwapchainManager::CreateSwapchainImageViews()
{
    auto &context = Context::Instance();
    auto &surfaceInfo = context.GetSurfaceInfo();
    auto &queueFamilyIndices = context.GetQueueFamilyIndicates();
    std::vector<vk::UniqueImageView> imageViews;
    auto images = GetSwapchainImages();
    for (auto &image : images)
    {
        vk::ImageViewCreateInfo imageViewCreateInfo;
        imageViewCreateInfo.setImage(image)
            .setViewType(vk::ImageViewType::e2D)
            .setFormat(surfaceInfo.format.format)
            .setComponents({vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity,
                            vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity})
            .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
        auto imageView = context.GetDevice().createImageViewUnique(imageViewCreateInfo);
        imageViews.push_back(std::move(imageView));
    }
    mSwapchainImageViews = std::move(imageViews);
    LogD("Swapchain Image Views Created.");
}

std::vector<vk::Image> SwapchainManager::GetSwapchainImages() const
{
    auto &context = Context::Instance();
    return context.GetDevice().getSwapchainImagesKHR(mSwapchain.get());
}

vk::SwapchainKHR SwapchainManager::GetSwapchain() const
{
    return mSwapchain.get();
}
std::vector<vk::ImageView> SwapchainManager::GetSwapchainImageViews() const
{
    std::vector<vk::ImageView> imageViews;
    for (auto &imageView : mSwapchainImageViews)
    {
        imageViews.push_back(imageView.get());
    }
    return imageViews;
}
} // namespace MEngine