#include "Context.hpp"

namespace MEngine
{
Context::~Context()
{
    mDevice->waitIdle();
    vmaDestroyAllocator(mVmaAllocator);
    mLogger->Debug("Context Destroyed");
}
Context::Context(std::shared_ptr<ILogger> logger, std::shared_ptr<IWindow> window) : mLogger(logger), mWindow(window)
{
    std::vector<const char *> instanceRequiredExtensions = mWindow->GetInstanceRequiredExtensions();
    std::vector<const char *> instanceRequiredLayers{"VK_LAYER_KHRONOS_validation",
                                                     "VK_LAYER_KHRONOS_synchronization2"};
    std::vector<const char *> deviceRequiredExtension;
    std::vector<const char *> deviceRequiredLayers;
    deviceRequiredExtension.push_back("VK_KHR_swapchain");
#ifdef PLATFORM_MACOS
    instanceRequiredExtensions.push_back("VK_KHR_portability_enumeration");
    deviceRequiredExtension.push_back("VK_KHR_portability_subset");
#endif
    mConfig.instanceRequiredExtensions = instanceRequiredExtensions;
    mConfig.instanceRequiredLayers = instanceRequiredLayers;
    mConfig.deviceRequiredExtensions = deviceRequiredExtension;
    mConfig.deviceRequiredLayers = deviceRequiredLayers;

    CreateInstance();
    PickPhysicalDevice();

    CreateSurface();
    QuerySurfaceInfo();

    QueryQueueFamilyIndicates();
    CreateDevice();

    GetQueues();
    CreateVmaAllocator();

    CreateSwapchain();
    CreateSwapchainImages();
    CreateSwapchainImageViews();

    mLogger->Debug("Context Created");
}

void Context::CreateInstance()
{
    vk::InstanceCreateInfo instanceCreateInfo;
    // layers and extensions
    auto layers = vk::enumerateInstanceLayerProperties();
    auto extensions = vk::enumerateInstanceExtensionProperties();

    vk::ApplicationInfo appInfo;
    // query instance max supported version
    mInstanceVersion = vk::enumerateInstanceVersion();
    auto variant = vk::apiVersionVariant(mInstanceVersion);
    auto major = vk::apiVersionMajor(mInstanceVersion);
    auto minor = vk::apiVersionMinor(mInstanceVersion);
    auto patch = vk::apiVersionPatch(mInstanceVersion);
    // set app info
    auto appVersion = vk::makeApiVersion(0, 0, 0, 1);
    appInfo.setPApplicationName("MEngine")
        .setApplicationVersion(appVersion)
        .setPEngineName({})
        .setEngineVersion({})
        .setApiVersion(mInstanceVersion);
    instanceCreateInfo.setFlags({})
        .setPApplicationInfo(&appInfo)
        .setPEnabledLayerNames(mConfig.instanceRequiredLayers)
        .setPEnabledExtensionNames(mConfig.instanceRequiredExtensions);
    mLogger->Trace("Instance Version: {}.{}.{}.{}", variant, major, minor, patch);
#ifdef PLATFORM_MACOS
    instanceCreateInfo.setFlags(vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR);
#endif
    mVKInstance = vk::createInstanceUnique(instanceCreateInfo);
    if (!mVKInstance)
    {
        mLogger->Debug("Failed to create instance");
        throw std::runtime_error("Failed to create instance");
    }
    // log
    for (auto &layer : mConfig.instanceRequiredLayers)
    {
        mLogger->Trace("Instance enabled layer: {}", layer);
    }
    for (auto &extension : mConfig.instanceRequiredExtensions)
    {
        mLogger->Trace("Instance enabled extension: {}", extension);
    }
    mLogger->Debug("Instance Created");
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
        mLogger->Debug("No physical devices found");
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
        mLogger->Debug("Physical Device Selected: {}", mPhysicalDevice.getProperties().deviceName.data());
    }
    else
    {
        mLogger->Debug("No suitable physical device found");
        throw std::runtime_error("No suitable physical device found");
    }
}
void Context::CreateSurface()
{
    auto surface = mWindow->GetSurface(mVKInstance.get());
    mSurface = vk::UniqueSurfaceKHR(surface, mVKInstance.get());
    if (!mSurface)
    {
        mLogger->Debug("Surface creation failed: No mSurface provided");
        throw std::runtime_error("Failed to create surface");
    }
    mLogger->Debug("Surface Created");
}

void Context::QuerySurfaceInfo()
{
    auto formats = mPhysicalDevice.getSurfaceFormatsKHR(mSurface.get());
    auto presentModes = mPhysicalDevice.getSurfacePresentModesKHR(mSurface.get());
    auto capabilities = mPhysicalDevice.getSurfaceCapabilitiesKHR(mSurface.get());
    std::vector<vk::SurfaceFormatKHR> candidatesFormats = {
        {vk::Format::eR32G32B32A32Sfloat, vk::ColorSpaceKHR::eSrgbNonlinear},
        {vk::Format::eR16G16B16A16Sfloat, vk::ColorSpaceKHR::eSrgbNonlinear},
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
    mLogger->Trace("Current Surface Info:");
    mLogger->Trace("Support Image Count: {}~{}", capabilities.minImageCount, capabilities.maxImageCount);
    mLogger->Trace("Support Array Layer: 1~{}", capabilities.maxImageArrayLayers);
    mLogger->Trace("Support Transforms", vk::to_string(capabilities.supportedTransforms));
    mLogger->Trace("Support Usage Flags", vk::to_string(capabilities.supportedUsageFlags));
    mLogger->Trace("Support CompositeAlpha {}", vk::to_string(capabilities.supportedCompositeAlpha));
    mLogger->Trace("Support Extent: {}x{}~{}x{}", capabilities.minImageExtent.width, capabilities.minImageExtent.height,
                   capabilities.maxImageExtent.width, capabilities.maxImageExtent.height);
    for (auto &supportFormat : formats)
    {
        mLogger->Trace("Support Format: {}", vk::to_string(supportFormat.format));
    }
    for (auto &supportPresentMode : presentModes)
    {
        mLogger->Trace("Support Present Mode: {}", vk::to_string(supportPresentMode));
    }
    mLogger->Debug("Current Format: {}", vk::to_string(mSurfaceInfo.format.format));
    mLogger->Debug("Current Color Space: {}", vk::to_string(mSurfaceInfo.format.colorSpace));
    mLogger->Debug("Current Present Mode: {}", vk::to_string(mSurfaceInfo.presentMode));
    mLogger->Debug("Current Extent: {}x{}", capabilities.currentExtent.width, capabilities.currentExtent.height);
    mLogger->Debug("Current Image Count: {}", mSurfaceInfo.imageCount);
    mLogger->Debug("Current Image Array Layer: {}", mSurfaceInfo.imageArrayLayer);
}

void Context::CreateSwapchain()
{
    vk::SwapchainCreateInfoKHR swapchainCreateInfo;
    auto queueFamilyIndicates = mQueueFamilyIndicates;
    swapchainCreateInfo.setSurface(mSurface.get())
        .setClipped(true)
        .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
        .setImageArrayLayers(mSurfaceInfo.imageArrayLayer)
        .setMinImageCount(mSurfaceInfo.imageCount)
        .setImageColorSpace(mSurfaceInfo.format.colorSpace)
        .setImageExtent(mSurfaceInfo.extent)
        .setImageFormat(mSurfaceInfo.format.format)
        .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
        .setPresentMode(mSurfaceInfo.presentMode)
        .setPreTransform(vk::SurfaceTransformFlagBitsKHR::eIdentity)
        .setMinImageCount(mSurfaceInfo.imageCount)
        .setOldSwapchain(nullptr);
    if (queueFamilyIndicates.graphicsFamily == queueFamilyIndicates.presentFamily)
    {
        swapchainCreateInfo.setImageSharingMode(vk::SharingMode::eExclusive)
            .setQueueFamilyIndices({queueFamilyIndicates.graphicsFamily.value()});
    }
    else
    {
        std::array<uint32_t, 2> queueFamilyIndicesArray = {queueFamilyIndicates.graphicsFamily.value(),
                                                           queueFamilyIndicates.presentFamily.value()};
        swapchainCreateInfo.setImageSharingMode(vk::SharingMode::eConcurrent)
            .setQueueFamilyIndices(queueFamilyIndicesArray);
    }
    mSwapchain = mDevice->createSwapchainKHRUnique(swapchainCreateInfo);
    if (!mSwapchain)
    {
        mLogger->Debug("Failed to create swapchain");
        throw std::runtime_error("Failed to create swapchain");
    }
    mLogger->Debug("Swapchain Created");
}

void Context::CreateSwapchainImages()
{
    auto swapchainImages = mDevice->getSwapchainImagesKHR(mSwapchain.get());
    for (auto &image : swapchainImages)
    {
        mSwapchainImages.push_back(image);
    }
    mLogger->Debug("Swapchain Images Created");
    mLogger->Trace("Swapchain Image Count: {}", mSwapchainImages.size());
}

void Context::CreateSwapchainImageViews()
{
    for (auto image : mSwapchainImages)
    {
        vk::ImageViewCreateInfo imageViewCreateInfo;
        imageViewCreateInfo.setImage(image)
            .setViewType(vk::ImageViewType::e2D)
            .setFormat(mSurfaceInfo.format.format)
            .setComponents(vk::ComponentMapping{})
            .setSubresourceRange(vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
        auto imageView = mDevice->createImageViewUnique(imageViewCreateInfo);
        if (!imageView)
        {
            mLogger->Debug("Failed to create image view for swapchain image");
            throw std::runtime_error("Failed to create image view for swapchain image");
        }
        mSwapchainImageViews.push_back(std::move(imageView));
    }
    mLogger->Debug("Swapchain Image Views Created");
    mLogger->Trace("Swapchain Image Count: {}", mSwapchainImageViews.size());
}
void Context::CreateDevice()
{
    auto layers = mPhysicalDevice.enumerateDeviceLayerProperties();
    auto extensions = mPhysicalDevice.enumerateDeviceExtensionProperties();

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

    std::set<uint32_t> uniqueQueueFamilies = {mQueueFamilyIndicates.graphicsFamily.value(),
                                              mQueueFamilyIndicates.presentFamily.value(),
                                              mQueueFamilyIndicates.transferFamily.value()};
    const float queuePriority = 1.0f;

    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        vk::DeviceQueueCreateInfo queueCreateInfo;
        queueCreateInfo.setQueueFamilyIndex(queueFamily).setQueueCount(1).setPQueuePriorities(&queuePriority);
        queueCreateInfos.push_back(queueCreateInfo);
    }

    vk::DeviceCreateInfo deviceCreateInfo;
    deviceCreateInfo.setQueueCreateInfos(queueCreateInfos)
        .setPEnabledExtensionNames(mConfig.deviceRequiredExtensions)
        .setPEnabledLayerNames(mConfig.deviceRequiredLayers)
        .setPEnabledFeatures(nullptr);

    mDevice = mPhysicalDevice.createDeviceUnique(deviceCreateInfo);
    if (!mDevice)
    {
        mLogger->Debug("Failed to create device");
        throw std::runtime_error("Failed to create device");
    }
    // log
    for (auto &layer : mConfig.deviceRequiredLayers)
    {
        mLogger->Trace("Device enabled layer: {}", layer);
    }
    for (auto &extension : mConfig.deviceRequiredExtensions)
    {
        mLogger->Trace("Device enabled extension: {}", extension);
    }
    mLogger->Debug("Device Created");
}
void Context::SetPresentQueueFamilyIndex(vk::SurfaceKHR surface)
{
    auto queueFamilyProperties = mPhysicalDevice.getQueueFamilyProperties();
    for (size_t i = 0; i < queueFamilyProperties.size(); i++)
    {
        auto &queueFamily = queueFamilyProperties[i];
        auto queueCount = queueFamily.queueCount;
        if (mPhysicalDevice.getSurfaceSupportKHR(i, surface))
        {
            mQueueFamilyIndicates.presentFamily = static_cast<uint32_t>(i);
            mLogger->Trace("Queue Family Index: {} Supports Presentation. Supports Queue Index:0~{}", i,
                           queueCount - 1);
        }
    }
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
            mLogger->Trace("Queue Family Index: {} Supports Graphics. Supports Queue Index:0~{}", i, queueCount - 1);
        }
        if (mPhysicalDevice.getSurfaceSupportKHR(i, mSurface.get()))
        {
            mQueueFamilyIndicates.presentFamily = static_cast<uint32_t>(i);
            mLogger->Trace("Queue Family Index: {} Supports Presentation. Supports Queue Index:0~{}", i,
                           queueCount - 1);
        }
        if (queueFamily.queueFlags & vk::QueueFlagBits::eTransfer)
        {
            mQueueFamilyIndicates.transferFamily = static_cast<uint32_t>(i);
            mQueueFamilyIndicates.transferFamilyCount = queueCount;
            mLogger->Trace("Queue Family Index: {} Supports Transfer. Supports Queue Index:0~{}", i, queueCount - 1);
        }
        if (mQueueFamilyIndicates.graphicsFamily.has_value() && mQueueFamilyIndicates.transferFamily.has_value())
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
    mLogger->Debug("Queues Getted");
}

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
void Context::SubmitToGraphicQueue(std::vector<vk::SubmitInfo> submits, vk::Fence fence)
{
    std::lock_guard<std::mutex> lock(mGraphicQueueMutex);
    mGraphicQueue.submit(submits, fence);
}
void Context::SubmitToPresnetQueue(vk::PresentInfoKHR presentInfo)
{
    std::lock_guard<std::mutex> lock(mPresentQueueMutex);
    auto result = mPresentQueue.presentKHR(presentInfo);
    if (result != vk::Result::eSuccess)
    {
        mLogger->Debug("Failed to present to the queue");
        throw std::runtime_error("Failed to present to the queue");
    }
}
void Context::SubmitToTransferQueue(std::vector<vk::SubmitInfo> submits, vk::Fence fence)
{
    std::lock_guard<std::mutex> lock(mTransferQueueMutex);
    mTransferQueue.submit(submits, fence);
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
    mLogger->Debug("VMA Allocator Created");
}
} // namespace MEngine