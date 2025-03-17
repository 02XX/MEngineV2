#include "Context.hpp"

namespace MEngine
{
Context &Context::Instance()
{
    static Context instance;
    return instance;
}
Context::~Context()
{
    Quit();
    LogD("Context Destroyed");
}
void Context::Quit()
{
    vmaDestroyAllocator(mVmaAllocator);
}
void Context::Init(std::function<vk::SurfaceKHR(vk::Instance)> createSurface,
                   const std::vector<const char *> instanceRequiredExtensions,
                   const std::vector<const char *> deviceRequiredExtensions,
                   const std::vector<const char *> deviceRequiredLayers,
                   const std::vector<const char *> instanceRequiredLayers)
{
    mVKInstanceEnabledExtensions = instanceRequiredExtensions;
    mVKDeviceEnabledExtensions = deviceRequiredExtensions;
    mVKDeviceEnabledLayers = deviceRequiredLayers;
    mVKInstanceEnabledLayers = instanceRequiredLayers;

    CreateInstance();
    PickPhysicalDevice();

    CreateSurface(createSurface);
    QuerySurfaceInfo();

    QueryQueueFamilyIndicates();
    CreateDevice();
    GetQueues();

    CreateVmaAllocator();
}

void Context::CreateInstance()
{
    vk::InstanceCreateInfo instanceCreateInfo;
    // layers and extensions
    auto layers = vk::enumerateInstanceLayerProperties();
    auto extensions = vk::enumerateInstanceExtensionProperties();

    vk::ApplicationInfo appInfo;
    // query instance max supported version
    auto instanceVersion = vk::enumerateInstanceVersion();
    auto variant = vk::apiVersionVariant(instanceVersion);
    auto major = vk::apiVersionMajor(instanceVersion);
    auto minor = vk::apiVersionMinor(instanceVersion);
    auto patch = vk::apiVersionPatch(instanceVersion);
    // set app info
    auto appVersion = vk::makeApiVersion(0, 0, 0, 1);
    appInfo.setPApplicationName("MEngine")
        .setApplicationVersion(appVersion)
        .setPEngineName({})
        .setEngineVersion({})
        .setApiVersion(instanceVersion);
    instanceCreateInfo.setFlags({})
        .setPApplicationInfo(&appInfo)
        .setPEnabledLayerNames(mVKInstanceEnabledLayers)
        .setPEnabledExtensionNames(mVKInstanceEnabledExtensions);
    LogT("Instance Version: {}.{}.{}.{}", variant, major, minor, patch);
    mVKInstance = vk::createInstanceUnique(instanceCreateInfo);

    // log
    for (auto &layer : mVKInstanceEnabledLayers)
    {
        LogT("Instance enabled layer: {}", layer);
    }
    for (auto &extension : mVKInstanceEnabledExtensions)
    {
        LogT("Instance enabled extension: {}", extension);
    }
    LogD("Instance Created");
}
void Context::CreateSurface(std::function<vk::SurfaceKHR(vk::Instance)> createSurface)
{
    mSurface = vk::UniqueSurfaceKHR(createSurface(mVKInstance.get()), mVKInstance.get());
    if (!mSurface)
    {
        LogE("Surface creation failed: No mSurface provided");
    }
    LogD("Surface Created");
}
void Context::QuerySurfaceInfo()
{
    auto formats = mPhysicalDevice.getSurfaceFormatsKHR(mSurface.get());
    auto presentModes = mPhysicalDevice.getSurfacePresentModesKHR(mSurface.get());
    auto capabilities = mPhysicalDevice.getSurfaceCapabilitiesKHR(mSurface.get());
    std::vector<vk::SurfaceFormatKHR> candidatesFormats = {
        {vk::Format::eR8G8B8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear},
        {vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear},
        {vk::Format::eR8G8B8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear},
        {vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear},
    };
    std::vector<vk::PresentModeKHR> candidatesPresentModes = {
        vk::PresentModeKHR::eMailbox,
        vk::PresentModeKHR::eFifo,
    };
    bool formatFound = false;
    bool presentModeFound = false;
    mSurfaceInfo.format = formats[0];
    for (auto &format : candidatesFormats)
    {
        for (auto &supportFormat : formats)
        {
            if (format.format == supportFormat.format && format.colorSpace == supportFormat.colorSpace)
            {
                mSurfaceInfo.format = format;
                formatFound = true;
                break;
            }
        }
        if (formatFound)
            break;
    }
    mSurfaceInfo.presentMode = presentModes[0];
    for (auto &presentMode : candidatesPresentModes)
    {
        for (auto &supportPresentMode : presentModes)
        {
            if (presentMode == supportPresentMode)
            {
                mSurfaceInfo.presentMode = presentMode;
                presentModeFound = true;
                break;
            }
        }
        if (presentModeFound)
            break;
    }
    mSurfaceInfo.extent = capabilities.currentExtent;
    mSurfaceInfo.imageCount = std::clamp(2u, capabilities.minImageCount, capabilities.maxImageCount);
    mSurfaceInfo.imageArrayLayer = std::clamp(1u, 1u, capabilities.maxImageArrayLayers);
    // log
    LogT("Current Surface Info:");
    LogT("Support Image Count: {}~{}", capabilities.minImageCount, capabilities.maxImageCount);
    LogT("Support Array Layer: 1~{}", capabilities.maxImageArrayLayers);
    LogT("Support Transforms", vk::to_string(capabilities.supportedTransforms));
    LogT("Support Usage Flags", vk::to_string(capabilities.supportedUsageFlags));
    LogT("Support CompositeAlpha {}", vk::to_string(capabilities.supportedCompositeAlpha));
    LogT("Support Extent: {}x{}~{}x{}", capabilities.minImageExtent.width, capabilities.minImageExtent.height,
         capabilities.maxImageExtent.width, capabilities.maxImageExtent.height);
    for (auto &supportFormat : formats)
    {
        LogT("Support Format: {}", vk::to_string(supportFormat.format));
    }
    for (auto &supportPresentMode : presentModes)
    {
        LogT("Support Present Mode: {}", vk::to_string(supportPresentMode));
    }
    LogD("Current Format: {}", vk::to_string(mSurfaceInfo.format.format));
    LogD("Current Color Space: {}", vk::to_string(mSurfaceInfo.format.colorSpace));
    LogD("Current Present Mode: {}", vk::to_string(mSurfaceInfo.presentMode));
    LogD("Current Extent: {}x{}", capabilities.currentExtent.width, capabilities.currentExtent.height);
    LogD("Current Image Count: {}", mSurfaceInfo.imageCount);
    LogD("Current Image Array Layer: {}", mSurfaceInfo.imageArrayLayer);
}

int Context::RatePhysicalDevices(vk::PhysicalDevice &mPhysicalDevice)
{
    auto properties = mPhysicalDevice.getProperties();
    auto features = mPhysicalDevice.getFeatures();
    int score = 0;
    // independent of the physical device
    if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
    {
        score += 1000;
    }
    score += properties.limits.maxImageDimension2D;
    return score;
}

void Context::PickPhysicalDevice()
{
    auto mPhysicalDevices = mVKInstance->enumeratePhysicalDevices();
    if (mPhysicalDevices.empty())
    {
        LogE("No physical devices found");
        throw std::runtime_error("No physical devices found");
    }
    std::multimap<int, vk::PhysicalDevice> candidates;
    for (auto &device : mPhysicalDevices)
    {
        int score = RatePhysicalDevices(device);
        candidates.insert(std::make_pair(score, device));
    }
    if (candidates.rbegin()->first > 0)
    {
        this->mPhysicalDevice = candidates.rbegin()->second;
        LogD("Physical Device Selected: {}", mPhysicalDevice.getProperties().deviceName.data());
    }
    else
    {
        LogE("No suitable physical device found");
        throw std::runtime_error("No suitable physical device found");
    }
}
void Context::CreateDevice()
{
    auto layers = mPhysicalDevice.enumerateDeviceLayerProperties();
    auto extensions = mPhysicalDevice.enumerateDeviceExtensionProperties();

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

    vk::DeviceQueueCreateInfo graphicQueueCreateInfo;
    vk::DeviceQueueCreateInfo presentQueueCreateInfo;
    vk::DeviceQueueCreateInfo transferQueueCreateInfo;
    const float graphicQueuePriority = 1.0f;
    const float presentQueuePriority = 1.0f;
    const float transferQueuePriority = 1.0f;
    // Graphics Queue
    graphicQueueCreateInfo.setQueueFamilyIndex(mQueueFamilyIndicates.graphicsFamily.value())
        .setQueueCount(1)
        .setPQueuePriorities(&graphicQueuePriority);
    queueCreateInfos.push_back(graphicQueueCreateInfo);

    // Present Queue
    presentQueueCreateInfo.setQueueFamilyIndex(mQueueFamilyIndicates.presentFamily.value())
        .setQueueCount(1)
        .setPQueuePriorities(&presentQueuePriority);

    // Transfer Queue
    transferQueueCreateInfo.setQueueFamilyIndex(mQueueFamilyIndicates.transferFamily.value())
        .setQueueCount(1)
        .setPQueuePriorities(&transferQueuePriority);

    vk::DeviceCreateInfo deviceCreateInfo;
    deviceCreateInfo.setQueueCreateInfos(queueCreateInfos)
        .setPEnabledExtensionNames(mVKDeviceEnabledExtensions)
        .setPEnabledLayerNames(mVKDeviceEnabledLayers)
        .setPEnabledFeatures(nullptr);

    mDevice = mPhysicalDevice.createDeviceUnique(deviceCreateInfo);

    // log
    for (auto &layer : mVKDeviceEnabledLayers)
    {
        LogT("Device enabled layer: {}", layer);
    }
    for (auto &extension : mVKDeviceEnabledExtensions)
    {
        LogT("Device enabled extension: {}", extension);
    }
    LogD("Device Created");
}
void Context::QueryQueueFamilyIndicates()
{
    auto queueFamilyProperties = mPhysicalDevice.getQueueFamilyProperties();
    for (size_t i = 0; i < queueFamilyProperties.size(); i++)
    {
        auto &queueFamily = queueFamilyProperties[i];
        auto queueCount = queueFamily.queueCount;
        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
        {
            mQueueFamilyIndicates.graphicsFamily = static_cast<uint32_t>(i);
            mQueueFamilyIndicates.graphicsFamilyCount = queueCount;
            LogT("Queue Family Index: {} Supports Graphics. Supports Queue Index:0~{}", i, queueCount - 1);
        }
        if (mPhysicalDevice.getSurfaceSupportKHR(i, mSurface.get()))
        {
            mQueueFamilyIndicates.presentFamily = static_cast<uint32_t>(i);
            LogT("Queue Family Index: {} Supports Presentation. Supports Queue Index:0~{}", i, queueCount - 1);
        }
        if (queueFamily.queueFlags & vk::QueueFlagBits::eTransfer)
        {
            mQueueFamilyIndicates.transferFamily = static_cast<uint32_t>(i);
            mQueueFamilyIndicates.transferFamilyCount = queueCount;
            LogT("Queue Family Index: {} Supports Transfer. Supports Queue Index:0~{}", i, queueCount - 1);
        }
        if (mQueueFamilyIndicates.graphicsFamily.has_value() && mQueueFamilyIndicates.presentFamily.has_value() &&
            mQueueFamilyIndicates.transferFamily.has_value())
        {
            break;
        }
    }
}
void Context::GetQueues()
{
    uint32_t graphicQueueIndex = 0;
    uint32_t presentQueueIndex = 0;
    uint32_t transferQueueIndex = 0;
    mGraphicQueue = mDevice->getQueue(mQueueFamilyIndicates.graphicsFamily.value(), graphicQueueIndex);
    mPresentQueue = mDevice->getQueue(mQueueFamilyIndicates.presentFamily.value(), presentQueueIndex);
    mTransferQueue = mDevice->getQueue(mQueueFamilyIndicates.transferFamily.value(), transferQueueIndex);
    LogD("Queues Getted");
}

// uint32_t Context::QueryMemory(uint32_t memoryTypeBits, vk::MemoryPropertyFlags property)
// {
//     auto memoryProperties = mPhysicalDevice.getMemoryProperties();
//     uint32_t index = -1;
//     for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
//     {
//         if ((i << 2) & memoryTypeBits && (property & memoryProperties.memoryTypes[i].propertyFlags))
//         {
//             index = i;
//             break;
//         }
//     }
//     if (index == -1)
//     {
//         LogE("No memory available type: {}, property: {}", memoryTypeBits, vk::to_string(property));
//         throw std::runtime_error("No memory available type");
//     }
//     return index;
// }

/*
https://www.reddit.com/r/vulkan/comments/umgs26/synchronize_queue_submission/?rdt=37374
Short answer - guard it with mutex and you already obey the rules of the queue usage. As queue is not tied to a thread
(like OpenGL context), vulkan docs just says that submit function is not thread safe.

Long answer: Do you really need to submit from different threads? What matters is multithreaded command buffer recording
as that’s the most demanding thing. As for the submission itself, that’s much cheaper thing. However, from vulkan
guidelines it is stated that you should submit no more than 10-20 command buffers per frame. Which also means you won’t
have 8 threads submitting concurrently 50-100 command buffers.

So then it means you can just have an std::mutex to do the submission. Or else you can pile up command buffers in a
vector and then call submit on it (all command buffers will be submitted in a single call as submit info takes pointer +
size).
*/
void Context::SubmitToGraphicQueue(std::vector<vk::SubmitInfo> submits, vk::UniqueFence &fence)
{
    mGraphicQueue.submit(submits, fence.get());
}
void Context::SubmitToPresnetQueue(vk::PresentInfoKHR presentInfo)
{
    auto result = mPresentQueue.presentKHR(presentInfo);
    if (result != vk::Result::eSuccess)
    {
        LogE("Failed to present to the queue");
        throw std::runtime_error("Failed to present to the queue");
    }
}
void Context::SubmitToTransferQueue(std::vector<vk::SubmitInfo> submits, vk::UniqueFence &fence)
{
    mTransferQueue.submit(submits, fence.get());
}
void Context::CreateVmaAllocator()
{
    // VmaVulkanFunctions vulkanFunctions = {};
    // vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
    // vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo allocatorCreateInfo{};
    allocatorCreateInfo.device = mDevice.get();
    allocatorCreateInfo.physicalDevice = mPhysicalDevice;
    allocatorCreateInfo.instance = mVKInstance.get();
    allocatorCreateInfo.flags =
        VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT | VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
    allocatorCreateInfo.vulkanApiVersion = vk::enumerateInstanceVersion();
    // allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;
    vmaCreateAllocator(&allocatorCreateInfo, &mVmaAllocator);
    LogD("VMA Allocator Created");
}

} // namespace MEngine