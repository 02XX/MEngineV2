#pragma once
#include "Logger.hpp"
#include "MEngine.hpp"
#include "VMA.hpp"
#include <vulkan/vulkan.hpp>

namespace MEngine
{
class MENGINE_API Context
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
    vk::Instance mVKInstance;
    vk::SurfaceKHR mSurface;
    vk::PhysicalDevice mPhysicalDevice;
    vk::Device mDevice;
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
    // void SubmitToGraphicQueue(std::vector<vk::SubmitInfo> submits, vk::Fence fence);
    // void SubmitToPresnetQueue(vk::PresentInfoKHR presentInfo);
    // void SubmitToTransferQueue(std::vector<vk::SubmitInfo> submits, vk::Fence fence);
};

} // namespace MEngine