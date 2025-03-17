#pragma once
#include "Logger.hpp"
#include "MEngine.hpp"
#include "VMA.hpp"
#include <vulkan/vulkan.hpp>

namespace MEngine
{
class MENGINE_API Context final
{
  private:
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
    } surfaceInfo;

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

    // std::mutex graphicQueueSubmitMutex;
    // std::mutex presentQueueSubmitMutex;
    // std::mutex transferQueueSubmitMutex;
    std::vector<const char *> mVKInstanceEnabledExtensions;
    std::vector<const char *> mVKInstanceEnabledLayers;
    std::vector<const char *> mVKDeviceEnabledExtensions;
    std::vector<const char *> mVKDeviceEnabledLayers;

  private:
    Context() = default;
    Context(const Context &) = delete;
    Context(Context &&) = delete;
    Context &operator=(const Context &) = delete;
    Context &operator=(Context &&) = delete;
    void CreateInstance();
    void QueryQueueFamilyIndicates();
    void QuerySwapchainInfo(uint32_t width, uint32_t height);
    void GetQueues();
    void PickPhysicalDevice();
    void CreateDevice();
    void QuerySurfaceInfo();
    uint32_t QueryMemory(uint32_t memoryTypeBits, vk::MemoryPropertyFlags property);
    void CreateSurface(std::function<vk::SurfaceKHR(vk::Instance)> createSurface);
    int RatePhysicalDevices(vk::PhysicalDevice &physicalDevice);
    void CreateVmaAllocator();

  public:
    static Context &Instance();
    ~Context();
    void Init(std::function<vk::SurfaceKHR(vk::Instance)> createSurface,
              const std::vector<const char *> instanceRequiredExtensions = {},
              const std::vector<const char *> deviceRequiredExtensions = {"VK_LAYER_KHRONOS_validation"},
              const std::vector<const char *> deviceRequiredLayers = {"VK_KHR_swapchain"},
              const std::vector<const char *> instanceRequiredLayers = {});
    void Quit();

    inline const vk::UniqueDevice &GetDevice() const
    {
        return mDevice;
    }
    inline const vk::PhysicalDevice GetPhysicalDevice() const
    {
        return mPhysicalDevice;
    }
    inline const VmaAllocator &GetVmaAllocator() const
    {
        return mVmaAllocator;
    }
    // void SubmitToGraphicQueue(std::vector<vk::SubmitInfo> submits, vk::Fence fence);
    // void SubmitToPresnetQueue(vk::PresentInfoKHR presentInfo);
    // void SubmitToTransferQueue(std::vector<vk::SubmitInfo> submits, vk::Fence fence);
};

} // namespace MEngine