#pragma once
#include "Logger.hpp"
#include "MEngine.hpp"
#include "VMA.hpp"
#include <cstdint>
#include <mutex>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace MEngine
{
class Context final
{
  private:
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
    struct SurfaceInfo
    {
        vk::SurfaceFormatKHR format;
        vk::Extent2D extent;
        vk::PresentModeKHR presentMode;
        uint32_t imageCount;
        uint32_t imageArrayLayer;
    };
    QueueFamilyIndicates mQueueFamilyIndicates;
    SurfaceInfo mSurfaceInfo;
    vk::UniqueInstance mVKInstance;
    vk::UniqueSurfaceKHR mSurface;
    vk::PhysicalDevice mPhysicalDevice;
    vk::UniqueDevice mDevice;
    vk::Queue mGraphicQueue;
    vk::Queue mPresentQueue;
    vk::Queue mTransferQueue;
    VmaAllocator mVmaAllocator;
    vk::UniqueSwapchainKHR mSwapchain;
    std::vector<vk::Image> mSwapchainImages;
    std::vector<vk::UniqueImageView> mSwapchainImageViews;
    std::vector<const char *> mVKInstanceEnabledExtensions;
    std::vector<const char *> mVKInstanceEnabledLayers;
    std::vector<const char *> mVKDeviceEnabledExtensions;
    std::vector<const char *> mVKDeviceEnabledLayers;

    std::mutex mGraphicQueueMutex;
    std::mutex mPresentQueueMutex;
    std::mutex mTransferQueueMutex;

  private:
    Context() = default;
    Context(const Context &) = delete;
    Context(Context &&) = delete;
    Context &operator=(const Context &) = delete;
    Context &operator=(Context &&) = delete;
    void CreateInstance();
    void QueryQueueFamilyIndicates();
    void GetQueues();
    void PickPhysicalDevice();
    void CreateDevice();
    void QuerySurfaceInfo();
    uint32_t QueryMemory(uint32_t memoryTypeBits, vk::MemoryPropertyFlags property);
    void CreateSurface(std::function<vk::SurfaceKHR(vk::Instance)> createSurface);
    int RatePhysicalDevices(vk::PhysicalDevice &physicalDevice);
    void CreateVmaAllocator();
    void CreateSwapchain();
    void CreateSwapchainImages();
    void CreateSwapchainImageViews();

  public:
    static Context &Instance();
    ~Context();
    void Init(std::function<vk::SurfaceKHR(vk::Instance)> createSurface,
              const std::vector<const char *> instanceRequiredExtensions = {},
              const std::vector<const char *> instanceRequiredLayers = {"VK_LAYER_KHRONOS_validation"},
              const std::vector<const char *> deviceRequiredExtensions = {"VK_KHR_swapchain"},
              const std::vector<const char *> deviceRequiredLayers = {});
    void Quit();
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
    inline const SurfaceInfo &GetSurfaceInfo() const
    {
        return mSurfaceInfo;
    }
    inline const vk::SurfaceKHR &GetSurface() const
    {
        return mSurface.get();
    }
    inline const vk::Instance &GetInstance() const
    {
        return mVKInstance.get();
    }
    inline const vk::SwapchainKHR &GetSwapchain() const
    {
        return mSwapchain.get();
    }
    inline const std::vector<vk::ImageView> &GetSwapchainImageViews() const
    {
        static std::vector<vk::ImageView> swapchainImageViewsRaw;
        swapchainImageViewsRaw.clear();
        for (const auto &uniqueImageView : mSwapchainImageViews)
        {
            swapchainImageViewsRaw.push_back(uniqueImageView.get());
        }
        return swapchainImageViewsRaw;
    }
    inline const std::vector<vk::Image> &GetSwapchainImages() const
    {
        return mSwapchainImages;
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
    void SubmitToGraphicQueue(std::vector<vk::SubmitInfo> submits, vk::Fence fence);
    void SubmitToPresnetQueue(vk::PresentInfoKHR presentInfo);
    void SubmitToTransferQueue(std::vector<vk::SubmitInfo> submits, vk::Fence fence);
};

} // namespace MEngine