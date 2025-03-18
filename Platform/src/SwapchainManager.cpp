#include "SwapchainManager.hpp"
#include <vector>

namespace MEngine
{
vk::UniqueSwapchainKHR SwapchainManager::CreateSwapchain(vk::Extent2D extent, vk::SurfaceKHR surface,
                                                         vk::SwapchainKHR oldSwapchain)
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

    auto swapchain = context.GetDevice()->createSwapchainKHRUnique(swapchainCreateInfo);
    LogD("Swapchain Created.");
    return swapchain;
}

std::vector<vk::UniqueImageView> SwapchainManager::CreateSwapchainImageViews(vk::SwapchainKHR swapchain)
{
    auto &context = Context::Instance();
    auto &surfaceInfo = context.GetSurfaceInfo();
    auto &queueFamilyIndices = context.GetQueueFamilyIndicates();
    std::vector<vk::UniqueImageView> imageViews;
    auto images = GetSwapchainImages(swapchain);
    for (auto &image : images)
    {
        vk::ImageViewCreateInfo imageViewCreateInfo;
        imageViewCreateInfo.setImage(image)
            .setViewType(vk::ImageViewType::e2D)
            .setFormat(surfaceInfo.format.format)
            .setComponents({vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity,
                            vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity})
            .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
        auto imageView = context.GetDevice()->createImageViewUnique(imageViewCreateInfo);
        imageViews.push_back(std::move(imageView));
    }
    return imageViews;
}

std::vector<vk::Image> SwapchainManager::GetSwapchainImages(vk::SwapchainKHR swapchain)
{
    auto &context = Context::Instance();
    auto images = context.GetDevice()->getSwapchainImagesKHR(swapchain);
    return images;
}
} // namespace MEngine