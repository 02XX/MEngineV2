#include "ImageManager.hpp"
#include "Image.hpp"
#include "Logger.hpp"
#include <vulkan/vulkan_structs.hpp>

namespace MEngine
{
ImageManager::ImageManager()
{
    mCommandBufferManager =
        std::make_unique<CommandBufferManager>(Context::Instance().GetQueueFamilyIndicates().transferFamily.value());
    mSyncPrimitiveManager = std::make_unique<SyncPrimitiveManager>();
    mBufferManager = std::make_unique<BufferManager>();
}
UniqueImage ImageManager::CreateUniqueTexture2D(vk::Extent2D extent, vk::Format format, uint32_t mipLevels,
                                                const void *data)
{
    auto &context = Context::Instance();
    vk::ImageCreateInfo imageCreateInfo{};
    imageCreateInfo.setImageType(vk::ImageType::e2D)
        .setFormat(format)
        .setExtent(vk::Extent3D(extent, 1))
        .setMipLevels(mipLevels)
        .setArrayLayers(1)
        .setSamples(vk::SampleCountFlagBits::e1)
        .setTiling(vk::ImageTiling::eOptimal)
        .setUsage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled |
                  vk::ImageUsageFlagBits::eTransferSrc)
        .setInitialLayout(vk::ImageLayout::eUndefined);
    auto image = std::make_unique<Image>(imageCreateInfo, VMA_MEMORY_USAGE_GPU_ONLY);
    if (data)
    {
        const vk::DeviceSize imageSize = extent.width * extent.height * GetFormatPixelSize(format);
        auto stagingBuffer = mBufferManager->CreateUniqueStagingBuffer(imageSize);
        TransitionLayout(image->GetImage(), vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
                         {vk::ImageAspectFlagBits::eColor, 0, mipLevels, 0, 1});
        CopyBufferToImage(stagingBuffer->GetBuffer(), image->GetImage(), extent);
        TransitionLayout(image->GetImage(), vk::ImageLayout::eTransferDstOptimal,
                         vk::ImageLayout::eShaderReadOnlyOptimal,
                         {vk::ImageAspectFlagBits::eColor, 0, mipLevels, 0, 1});
    }
    LogD("Created Texture2D with {}x{}, format: {}", extent.width, extent.height, vk::to_string(format));
    return image;
}

SharedImage ImageManager::CreateSharedTexture2D(vk::Extent2D extent, vk::Format format, uint32_t mipLevels,
                                                const void *data)
{
    auto &context = Context::Instance();
    vk::ImageCreateInfo imageCreateInfo{};
    imageCreateInfo.setImageType(vk::ImageType::e2D)
        .setFormat(format)
        .setExtent(vk::Extent3D(extent, 1))
        .setMipLevels(mipLevels)
        .setArrayLayers(1)
        .setSamples(vk::SampleCountFlagBits::e1)
        .setTiling(vk::ImageTiling::eOptimal)
        .setUsage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled |
                  vk::ImageUsageFlagBits::eTransferSrc)
        .setInitialLayout(vk::ImageLayout::eUndefined);
    auto image = std::make_shared<Image>(imageCreateInfo, VMA_MEMORY_USAGE_GPU_ONLY);
    if (data)
    {
        const vk::DeviceSize imageSize = extent.width * extent.height * GetFormatPixelSize(format);
        auto stagingBuffer = mBufferManager->CreateUniqueStagingBuffer(imageSize);
        TransitionLayout(image->GetImage(), vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
                         {vk::ImageAspectFlagBits::eColor, 0, mipLevels, 0, 1});
        CopyBufferToImage(stagingBuffer->GetBuffer(), image->GetImage(), extent);
        TransitionLayout(image->GetImage(), vk::ImageLayout::eTransferDstOptimal,
                         vk::ImageLayout::eShaderReadOnlyOptimal,
                         {vk::ImageAspectFlagBits::eColor, 0, mipLevels, 0, 1});
    }
    LogD("Created Texture2D with {}x{}, format: {}", extent.width, extent.height, vk::to_string(format));
    return image;
}

UniqueImage ImageManager::CreateUniqueRenderTarget(vk::Extent2D extent, vk::Format format, vk::ImageUsageFlags usage,
                                                   uint32_t mipLevels, vk::SampleCountFlagBits samples)
{
    vk::ImageCreateInfo imageCreateInfo{};
    imageCreateInfo.setImageType(vk::ImageType::e2D)
        .setFormat(format)
        .setExtent(vk::Extent3D(extent, 1))
        .setMipLevels(mipLevels)
        .setArrayLayers(1)
        .setSamples(vk::SampleCountFlagBits::e1)
        .setTiling(vk::ImageTiling::eOptimal)
        .setUsage(usage)
        .setInitialLayout(vk::ImageLayout::eUndefined);
    auto image = std::make_unique<Image>(imageCreateInfo, VMA_MEMORY_USAGE_GPU_ONLY);
    TransitionLayout(image->GetImage(), vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal,
                     {vk::ImageAspectFlagBits::eColor, 0, mipLevels, 0, 1});
    return image;
}
SharedImage ImageManager::CreateSharedRenderTarget(vk::Extent2D extent, vk::Format format, vk::ImageUsageFlags usage,
                                                   uint32_t mipLevels, vk::SampleCountFlagBits samples)
{
    vk::ImageCreateInfo imageCreateInfo{};
    imageCreateInfo.setImageType(vk::ImageType::e2D)
        .setFormat(format)
        .setExtent(vk::Extent3D(extent, 1))
        .setMipLevels(mipLevels)
        .setArrayLayers(1)
        .setSamples(vk::SampleCountFlagBits::e1)
        .setTiling(vk::ImageTiling::eOptimal)
        .setUsage(usage)
        .setInitialLayout(vk::ImageLayout::eUndefined);
    auto image = std::make_shared<Image>(imageCreateInfo, VMA_MEMORY_USAGE_GPU_ONLY);
    TransitionLayout(image->GetImage(), vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal,
                     {vk::ImageAspectFlagBits::eColor, 0, mipLevels, 0, 1});
    return image;
}

UniqueImage ImageManager::CreateUniqueDepthStencil(vk::Extent2D extent, vk::Format format, uint32_t mipLevels,
                                                   vk::SampleCountFlagBits samples)
{
    vk::ImageCreateInfo imageCreateInfo{};
    imageCreateInfo.setImageType(vk::ImageType::e2D)
        .setFormat(format)
        .setExtent(vk::Extent3D(extent, 1))
        .setMipLevels(mipLevels)
        .setArrayLayers(1)
        .setSamples(samples)
        .setTiling(vk::ImageTiling::eOptimal)
        .setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
        .setInitialLayout(vk::ImageLayout::eUndefined);
    auto image = std::make_unique<Image>(imageCreateInfo, VMA_MEMORY_USAGE_GPU_ONLY);
    TransitionLayout(image->GetImage(), vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal,
                     {vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, mipLevels, 0, 1});
    return image;
}
SharedImage ImageManager::CreateSharedDepthStencil(vk::Extent2D extent, vk::Format format, uint32_t mipLevels,
                                                   vk::SampleCountFlagBits samples)
{
    vk::ImageCreateInfo imageCreateInfo{};
    imageCreateInfo.setImageType(vk::ImageType::e2D)
        .setFormat(format)
        .setExtent(vk::Extent3D(extent, 1))
        .setMipLevels(mipLevels)
        .setArrayLayers(1)
        .setSamples(samples)
        .setTiling(vk::ImageTiling::eOptimal)
        .setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
        .setInitialLayout(vk::ImageLayout::eUndefined);
    auto image = std::make_shared<Image>(imageCreateInfo, VMA_MEMORY_USAGE_GPU_ONLY);
    TransitionLayout(image->GetImage(), vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal,
                     {vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, mipLevels, 0, 1});
    return image;
}

UniqueImage ImageManager::CreateUniqueStorageImage(vk::Extent2D extent, vk::Format format, uint32_t mipLevels,
                                                   vk::SampleCountFlagBits samples)
{
    vk::ImageCreateInfo imageCreateInfo{};
    imageCreateInfo.setImageType(vk::ImageType::e2D)
        .setFormat(format)
        .setExtent(vk::Extent3D(extent, 1))
        .setMipLevels(mipLevels)
        .setArrayLayers(1)
        .setSamples(samples)
        .setTiling(vk::ImageTiling::eOptimal)
        .setUsage(vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled)
        .setInitialLayout(vk::ImageLayout::eUndefined);

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    auto image = std::make_unique<Image>(imageCreateInfo, VMA_MEMORY_USAGE_GPU_ONLY);
    TransitionLayout(image->GetImage(), vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
                     {vk::ImageAspectFlagBits::eColor, 0, mipLevels, 0, 1});
    return image;
}
SharedImage ImageManager::CreateSharedStorageImage(vk::Extent2D extent, vk::Format format, uint32_t mipLevels,
                                                   vk::SampleCountFlagBits samples)
{
    vk::ImageCreateInfo imageCreateInfo{};
    imageCreateInfo.setImageType(vk::ImageType::e2D)
        .setFormat(format)
        .setExtent(vk::Extent3D(extent, 1))
        .setMipLevels(mipLevels)
        .setArrayLayers(1)
        .setSamples(samples)
        .setTiling(vk::ImageTiling::eOptimal)
        .setUsage(vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled)
        .setInitialLayout(vk::ImageLayout::eUndefined);

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    auto image = std::make_shared<Image>(imageCreateInfo, VMA_MEMORY_USAGE_GPU_ONLY);
    TransitionLayout(image->GetImage(), vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
                     {vk::ImageAspectFlagBits::eColor, 0, mipLevels, 0, 1});
    return image;
}

void ImageManager::CopyBufferToImage(vk::Buffer srcBuffer, vk::Image dstImage, vk::Extent2D extent,
                                     vk::ImageSubresourceLayers imageSubresourceLayers)
{
    auto &context = Context::Instance();
    auto commandBuffer = mCommandBufferManager->CreatePrimaryCommandBuffer();

    // vk::ImageSubresourceLayers imageSubresourceLayers;
    // imageSubresourceLayers
    //     .setAspectMask(aspectMask)         // 图像方面（颜色/深度等）
    //     .setMipLevel(mipLevels)            // 目标mip层级
    //     .setBaseArrayLayer(baseArrayLayer) // 起始数组层
    //     .setLayerCount(layerCount);        // 拷贝层数
    vk::BufferImageCopy region{};
    region
        .setBufferOffset(0)                    // 缓冲区的起始字节偏移
        .setBufferRowLength(0)                 // 缓冲区中每行的像素数（内存布局） 0-和extent一样
        .setBufferImageHeight(0)               // 缓冲区中每列的像素行数（内存布局） 0-和extent一样
        .setImageOffset(vk::Offset3D(0, 0, 0)) // 图像起始坐标（x,y,z）
        .setImageExtent(vk::Extent3D(extent, 1)) // 拷贝的区域尺寸
        .setImageSubresource(imageSubresourceLayers);
    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    commandBuffer->begin(beginInfo);
    commandBuffer->copyBufferToImage(srcBuffer, dstImage, vk::ImageLayout::eTransferDstOptimal, {region});
    commandBuffer->end();
    auto fence = mSyncPrimitiveManager->CreateFence();
    vk::SubmitInfo submitInfo;
    submitInfo.setCommandBuffers(commandBuffer.get());
    context.SubmitToTransferQueue({submitInfo}, fence);
    auto result = context.GetDevice()->waitForFences(fence.get(), vk::True, 1'000'000'000);
    if (result != vk::Result::eSuccess)
    {
        LogE("Copy image operation failed");
        throw std::runtime_error("Copy image operation failed");
    }
    LogD("Copy image operation success");
}

void ImageManager::TransitionLayout(vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
                                    vk::ImageSubresourceRange subresourceRange)
{
    auto &context = Context::Instance();
    auto commandBuffer = mCommandBufferManager->CreatePrimaryCommandBuffer();

    vk::ImageMemoryBarrier barrier{};
    barrier.setImage(image)
        .setOldLayout(oldLayout)
        .setNewLayout(newLayout)
        .setSrcQueueFamilyIndex(0)
        .setDstQueueFamilyIndex(0)
        .setSubresourceRange(subresourceRange);

    // 根据布局转换类型设置访问掩码
    std::tie(barrier.srcAccessMask, barrier.dstAccessMask) = GetAccessMasksForLayout(oldLayout, newLayout);

    // 根据布局转换类型设置管线阶段
    auto [srcStage, dstStage] = GetPipelineStagesForLayout(oldLayout, newLayout);

    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    commandBuffer->begin(beginInfo);
    commandBuffer->pipelineBarrier(srcStage, dstStage, {}, // 无特殊依赖标志
                                   {},                     // 无内存屏障
                                   {},                     // 无缓冲区屏障
                                   barrier                 // 单图像屏障
    );
    commandBuffer->end();
    auto fence = mSyncPrimitiveManager->CreateFence();
    vk::SubmitInfo submitInfo;
    submitInfo.setCommandBuffers(commandBuffer.get());
    context.SubmitToTransferQueue({submitInfo}, fence);
    auto result = context.GetDevice()->waitForFences(fence.get(), vk::True, 1'000'000'000);
    if (result != vk::Result::eSuccess)
    {
        LogE("Transition layout operation failed");
        throw std::runtime_error("Transition layout operation failed");
    }
    LogD("Transition layout operation failed");
}

uint32_t ImageManager::GetFormatPixelSize(vk::Format format) const
{
    switch (format)
    {
    case vk::Format::eR8G8B8A8Unorm:
        return 4;
    case vk::Format::eR32G32B32A32Sfloat:
        return 16;
    case vk::Format::eD32Sfloat:
        return 4;
    default:
        throw std::runtime_error("Unsupported format");
    }
}
std::pair<vk::AccessFlags, vk::AccessFlags> ImageManager::GetAccessMasksForLayout(vk::ImageLayout oldLayout,
                                                                                  vk::ImageLayout newLayout)
{
    using enum vk::AccessFlagBits;
    using enum vk::ImageLayout;

    if (oldLayout == eUndefined && newLayout == eTransferDstOptimal)
    {
        return {eNone, eTransferWrite};
    }
    else if (oldLayout == eTransferDstOptimal && newLayout == eShaderReadOnlyOptimal)
    {
        return {eTransferWrite, eShaderRead};
    }
    else if (oldLayout == eUndefined && newLayout == eDepthStencilAttachmentOptimal)
    {
        return {eNone, eDepthStencilAttachmentWrite};
    }
    else if (oldLayout == eUndefined && newLayout == eColorAttachmentOptimal)
    {
        return {eNone, eColorAttachmentWrite};
    }
    else
    {
        LogE("Unsupported layout transition");
        throw std::runtime_error("Unsupported layout transition");
    }
}
std::pair<vk::PipelineStageFlags, vk::PipelineStageFlags> ImageManager::GetPipelineStagesForLayout(
    vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
{
    using enum vk::PipelineStageFlagBits;
    using enum vk::ImageLayout;

    if (oldLayout == eUndefined)
    {
        if (newLayout == eTransferDstOptimal)
        {
            return {eTopOfPipe, eTransfer};
        }
        else if (newLayout == eDepthStencilAttachmentOptimal)
        {
            return {eTopOfPipe, eEarlyFragmentTests};
        }
    }
    else if (oldLayout == eTransferDstOptimal && newLayout == eShaderReadOnlyOptimal)
    {
        return {eTransfer, eFragmentShader};
    }
    else
    {
        LogE("Unsupported layout transition");
        throw std::runtime_error("Unsupported layout transition");
    }
}
vk::UniqueImageView ImageManager::CreateImageView(vk::Image image, vk::Format format, vk::ComponentMapping components,
                                                  vk::ImageSubresourceRange subresourceRange)
{
    auto &context = Context::Instance();
    vk::ImageViewCreateInfo imageViewCreateInfo;
    imageViewCreateInfo.setImage(image)
        .setViewType(vk::ImageViewType::e2D)
        .setFormat(format)
        .setComponents(components)
        .setSubresourceRange(subresourceRange);
    auto imageView = context.GetDevice()->createImageViewUnique(imageViewCreateInfo);
    LogD("Image view created");
    return imageView;
}
} // namespace MEngine
