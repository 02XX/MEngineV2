#pragma once
#include "Interface/ILogger.hpp"
#include "Interface/IWindow.hpp"
#include "MEngine.hpp"
#include "NoCopyable.hpp"
#include "VMA.hpp"
#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace MEngine
{
struct ContextConfig
{

    std::vector<const char *> instanceRequiredExtensions;
    std::vector<const char *> instanceRequiredLayers;
    std::vector<const char *> deviceRequiredExtensions;
    std::vector<const char *> deviceRequiredLayers;
};
class Context final : public NoCopyable
{
  private:
    // DI
    std::shared_ptr<ILogger> mLogger;
    std::shared_ptr<IWindow> mWindow;

  private:
    ContextConfig mConfig;

    uint32_t mInstanceVersion = 0;
    struct QueueFamilyIndicates
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> graphicsFamilyCount;
        std::optional<uint32_t> presentFamily;
        std::optional<uint32_t> presentFamilyCount;
        std::optional<uint32_t> transferFamily;
        std::optional<uint32_t> transferFamilyCount;
    };
    QueueFamilyIndicates mQueueFamilyIndicates;
    vk::UniqueInstance mVKInstance;
    vk::PhysicalDevice mPhysicalDevice;
    vk::UniqueDevice mDevice;
    vk::Queue mGraphicQueue;
    vk::Queue mPresentQueue;
    vk::Queue mTransferQueue;
    VmaAllocator mVmaAllocator;
    // surface
    struct SurfaceInfo
    {
        vk::SurfaceFormatKHR format;
        vk::Extent2D extent;
        vk::PresentModeKHR presentMode;
        uint32_t imageCount;
        uint32_t imageArrayLayer;
    } mSurfaceInfo;
    vk::UniqueSurfaceKHR mSurface;
    // swapchain
    vk::UniqueSwapchainKHR mSwapchain;
    std::vector<vk::Image> mSwapchainImages;
    std::vector<vk::UniqueImageView> mSwapchainImageViews;

    std::mutex mGraphicQueueMutex;
    std::mutex mPresentQueueMutex;
    std::mutex mTransferQueueMutex;

  private:
    void CreateInstance();
    void QueryQueueFamilyIndicates();
    void GetQueues();
    void PickPhysicalDevice();
    void CreateDevice();
    int RatePhysicalDevices(vk::PhysicalDevice &physicalDevice);
    void CreateVmaAllocator();

    void CreateSurface();
    void QuerySurfaceInfo();

    void CreateSwapchain();
    void CreateSwapchainImages();
    void CreateSwapchainImageViews();

  public:
    Context(std::shared_ptr<ILogger> logger, std::shared_ptr<IWindow> window, ContextConfig config);
    ~Context();
    void SetPresentQueueFamilyIndex(vk::SurfaceKHR surface);
    inline uint32_t GetInstanceVersion() const
    {
        return mInstanceVersion;
    }
    inline const vk::Device &GetDevice() const
    {
        return mDevice.get();
    }
    inline const vk::PhysicalDevice GetPhysicalDevice() const
    {
        return mPhysicalDevice;
    }
    inline const VmaAllocator &GetVmaAllocator() const
    {
        return mVmaAllocator;
    }
    inline const QueueFamilyIndicates &GetQueueFamilyIndicates() const
    {
        return mQueueFamilyIndicates;
    }
    inline const vk::Instance &GetInstance() const
    {
        return mVKInstance.get();
    }
    inline const vk::Queue &GetGraphicQueue() const
    {
        return mGraphicQueue;
    }
    inline const vk::Queue &GetPresentQueue() const
    {
        return mPresentQueue;
    }
    inline const vk::Queue &GetTransferQueue() const
    {
        return mTransferQueue;
    }
    inline const vk::SurfaceKHR &GetSurface() const
    {
        return mSurface.get();
    }
    inline const vk::SwapchainKHR &GetSwapchain() const
    {
        return mSwapchain.get();
    }
    inline const std::vector<vk::Image> &GetSwapchainImages() const
    {
        return mSwapchainImages;
    }
    inline std::vector<vk::ImageView> GetSwapchainImageViews() const
    {
        std::vector<vk::ImageView> imageViews;
        imageViews.reserve(mSwapchainImageViews.size());
        for (const auto &uniqueImageView : mSwapchainImageViews)
        {
            imageViews.push_back(uniqueImageView.get());
        }
        return imageViews;
    }
    inline const SurfaceInfo &GetSurfaceInfo() const
    {
        return mSurfaceInfo;
    }
    void SubmitToGraphicQueue(std::vector<vk::SubmitInfo> submits, vk::Fence fence);
    void SubmitToPresnetQueue(vk::PresentInfoKHR presentInfo);
    void SubmitToTransferQueue(std::vector<vk::SubmitInfo> submits, vk::Fence fence);
};

} // namespace MEngine