#include "ImageFactory.hpp"

namespace MEngine
{
ImageFactory::ImageFactory(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                           std::shared_ptr<CommandBufferManager> commandBufferManager,
                           std::shared_ptr<SyncPrimitiveManager> syncPrimitiveManager,
                           std::shared_ptr<BufferFactory> bufferFactory)
    : mContext(context), mLogger(logger), mCommandBufferManager(commandBufferManager),
      mSyncPrimitiveManager(syncPrimitiveManager), mBufferFactory(bufferFactory)
{
    mFence = mSyncPrimitiveManager->CreateFence();
    mCopyCommandBuffer = mCommandBufferManager->CreatePrimaryCommandBuffer(CommandBufferType::Transfer);
    mPreTransitionCommandBuffer = mCommandBufferManager->CreatePrimaryCommandBuffer(CommandBufferType::Transfer);
    mPostTransitionCommandBuffer = mCommandBufferManager->CreatePrimaryCommandBuffer(CommandBufferType::Transfer);
    mPreTransitionDone = mSyncPrimitiveManager->CreateUniqueSemaphore();
    mPostTransitionDone = mSyncPrimitiveManager->CreateUniqueSemaphore();
    mCopyDone = mSyncPrimitiveManager->CreateUniqueSemaphore();

    mTexture2DFormats = {
        vk::Format::eR8G8B8A8Srgb,       // HDR
        vk::Format::eR32G32B32A32Sfloat, // HDR
        vk::Format::eR16G16B16A16Sfloat, // HDR
        vk::Format::eB8G8R8A8Srgb,       vk::Format::eR8G8B8A8Unorm,
    };
    mTextureCubeFormats = {
        vk::Format::eR32G32B32A32Sfloat, // HDR
        vk::Format::eR16G16B16A16Sfloat, // HDR
        vk::Format::eR8G8B8A8Srgb,       // HDR
        vk::Format::eB8G8R8A8Srgb,       vk::Format::eR8G8B8A8Unorm,
    };
    mRenderTargetFormats = {mContext->GetSurfaceInfo().format.format};
    mDepthStencilCandidintFormats = {
        vk::Format::eD32SfloatS8Uint,
        vk::Format::eD24UnormS8Uint,
    };
    mStorageFormats = {
        vk::Format::eR32G32B32A32Sfloat, // HDR
        vk::Format::eR16G16B16A16Sfloat, // HDR
        vk::Format::eR8G8B8A8Srgb,       // HDR
        vk::Format::eB8G8R8A8Srgb,       vk::Format::eR8G8B8A8Unorm,
    };
    QueryImageFormat();
}
UniqueImage ImageFactory::CreateImage(ImageType type, vk::Extent3D extent, uint32_t mipLevels,
                                      vk::SampleCountFlagBits samples)
{
    return CreateImage(type, extent, 0, nullptr, mipLevels, samples);
}
UniqueImage ImageFactory::CreateImage(ImageType type, vk::Extent3D extent, vk::DeviceSize size, const void *data,
                                      uint32_t mipLevels, vk::SampleCountFlagBits samples)
{
    uint32_t arrayLayers{};
    VmaMemoryUsage memoryUsage{};
    VmaAllocationCreateFlags createflags{};
    vk::ImageUsageFlags imageUsage{};
    vk::ImageTiling tiling{};
    vk::ImageType imageType{};

    vk::ImageLayout imageLayout = vk::ImageLayout::eUndefined;
    vk::AccessFlags accessMask = vk::AccessFlagBits::eNoneKHR;
    vk::PipelineStageFlagBits pipelineStage = vk::PipelineStageFlagBits::eTopOfPipe;
    switch (type)
    {
    case ImageType::Texture2D:
        arrayLayers = 1;
        imageType = vk::ImageType::e2D;
        imageUsage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst |
                     vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eColorAttachment |
                     vk::ImageUsageFlagBits::eInputAttachment;
        tiling = vk::ImageTiling::eOptimal;
        memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
        imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        accessMask = vk::AccessFlagBits::eShaderRead;
        pipelineStage = vk::PipelineStageFlagBits::eFragmentShader;
        break;
    case ImageType::TextureCube:
        imageType = vk::ImageType::e2D;
        arrayLayers = 6;
        imageUsage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst |
                     vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eColorAttachment |
                     vk::ImageUsageFlagBits::eInputAttachment;
        tiling = vk::ImageTiling::eOptimal;
        memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
        imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        accessMask = vk::AccessFlagBits::eShaderRead;
        pipelineStage = vk::PipelineStageFlagBits::eFragmentShader;
        break;
    case ImageType::RenderTarget:
        imageType = vk::ImageType::e2D;
        arrayLayers = 1;
        imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment |
                     vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eSampled;
        tiling = vk::ImageTiling::eOptimal;
        memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
        imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
        accessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead;
        pipelineStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        break;
    case ImageType::DepthStencil:
        imageType = vk::ImageType::e2D;
        arrayLayers = 1;
        imageUsage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc |
                     vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eInputAttachment;
        tiling = vk::ImageTiling::eOptimal;
        memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
        imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
        accessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentRead;
        pipelineStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
        break;
    case ImageType::Storage:
        imageType = vk::ImageType::e2D;
        arrayLayers = 1;
        imageUsage = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled |
                     vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst;
        tiling = vk::ImageTiling::eOptimal;
        memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
        imageLayout = vk::ImageLayout::eGeneral;
        accessMask = vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite;
        pipelineStage = vk::PipelineStageFlagBits::eComputeShader;
        break;
    default:
        mLogger->Error("Invalid image type");
        throw std::invalid_argument("Invalid image type");
    }
    vk::ImageCreateInfo imageCreateInfo{};
    imageCreateInfo.setImageType(imageType)
        .setFormat(GetBestFormat(type))
        .setExtent(extent)
        .setMipLevels(mipLevels)
        .setArrayLayers(arrayLayers)
        .setSamples(samples)
        .setTiling(tiling)
        .setUsage(imageUsage)
        .setInitialLayout(vk::ImageLayout::eUndefined);
    auto image = std::make_unique<Image>(mContext, imageCreateInfo, memoryUsage, createflags);
    if (data)
    {
        if (type != ImageType::DepthStencil)
        {
            mContext->GetDevice().resetFences({mFence.get()});
            // 转换图像布局到eTransferDst
            mPreTransitionCommandBuffer->reset();
            mPreTransitionCommandBuffer->begin(vk::CommandBufferBeginInfo{});
            vk::ImageMemoryBarrier preBarrier{};
            preBarrier.setImage(image->GetHandle())
                .setOldLayout(vk::ImageLayout::eUndefined)
                .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
                .setSrcAccessMask(vk::AccessFlagBits::eNoneKHR)
                .setDstAccessMask(vk::AccessFlagBits::eTransferWrite)
                .setSrcQueueFamilyIndex(mContext->GetQueueFamilyIndicates().graphicsFamily.value())
                .setDstQueueFamilyIndex(mContext->GetQueueFamilyIndicates().graphicsFamily.value())
                .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, mipLevels, 0, arrayLayers});
            mPreTransitionCommandBuffer->pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe,
                                                         vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, preBarrier);
            mPreTransitionCommandBuffer->end();
            vk::SubmitInfo preSubmitInfo;
            std::vector<vk::PipelineStageFlags> waitDstStageMask = {vk::PipelineStageFlagBits::eTransfer};
            preSubmitInfo.setCommandBuffers(mPreTransitionCommandBuffer.get())
                .setSignalSemaphores({mPreTransitionDone.get()});

            // 复制数据到图像
            auto buffer = mBufferFactory->CreateBuffer(BufferType::Staging, size, data);
            vk::ImageSubresourceLayers imageSubresourceLayers{};
            imageSubresourceLayers.setAspectMask(vk::ImageAspectFlagBits::eColor)
                .setMipLevel(0)
                .setBaseArrayLayer(0)
                .setLayerCount(arrayLayers);
            mCopyCommandBuffer->reset();
            vk::BufferImageCopy region{};
            region.setBufferOffset(0)
                .setBufferRowLength(0)
                .setBufferImageHeight(0)
                .setImageOffset(vk::Offset3D(0, 0, 0))
                .setImageExtent(extent)
                .setImageSubresource(imageSubresourceLayers);
            vk::CommandBufferBeginInfo beginInfo;
            beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
            mCopyCommandBuffer->begin(beginInfo);
            mCopyCommandBuffer->copyBufferToImage(buffer->GetHandle(), image->GetHandle(),
                                                  vk::ImageLayout::eTransferDstOptimal, {region});
            mCopyCommandBuffer->end();

            vk::SubmitInfo copySubmitInfo;
            copySubmitInfo.setCommandBuffers(mCopyCommandBuffer.get())
                .setWaitSemaphores({mPreTransitionDone.get()})
                .setSignalSemaphores({mCopyDone.get()})
                .setWaitDstStageMask(waitDstStageMask);
            // 转换图像布局
            mPostTransitionCommandBuffer->reset();
            mPostTransitionCommandBuffer->begin(vk::CommandBufferBeginInfo{});
            vk::ImageMemoryBarrier postBarrier{};
            postBarrier.setImage(image->GetHandle())
                .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
                .setNewLayout(imageLayout)
                .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
                .setDstAccessMask(accessMask)
                .setSrcQueueFamilyIndex(mContext->GetQueueFamilyIndicates().graphicsFamily.value())
                .setDstQueueFamilyIndex(mContext->GetQueueFamilyIndicates().graphicsFamily.value())
                .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, mipLevels, 0, arrayLayers});
            mPostTransitionCommandBuffer->pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, pipelineStage, {}, {},
                                                          {}, postBarrier);
            mPostTransitionCommandBuffer->end();
            vk::SubmitInfo postSubmitInfo;
            postSubmitInfo.setCommandBuffers(mPostTransitionCommandBuffer.get())
                .setWaitSemaphores({mCopyDone.get()})
                .setWaitDstStageMask(waitDstStageMask);
            mContext->SubmitToTransferQueue({preSubmitInfo, copySubmitInfo, postSubmitInfo}, mFence.get());
            auto result = mContext->GetDevice().waitForFences(mFence.get(), vk::True, 1'000'000'000);
            if (result != vk::Result::eSuccess)
            {
                mLogger->Error("Transition image layout operation failed");
                throw std::runtime_error("Transition image layout operation failed");
            }
        }
    }
    return image;
}
void ImageFactory::CopyBufferToImage(Buffer *srcBuffer, Image *dstImage,
                                     vk::ImageSubresourceLayers imageSubresourceLayers,
                                     const std::vector<vk::Semaphore> &waitSemaphores,
                                     const std::vector<vk::Semaphore> &signalSemaphores,
                                     const std::vector<vk::PipelineStageFlags> &waitDstStageMask, vk::Fence fences)
{
    mCopyCommandBuffer->reset();
    vk::BufferImageCopy region{};
    region.setBufferOffset(0)
        .setBufferRowLength(0)
        .setBufferImageHeight(0)
        .setImageOffset(vk::Offset3D(0, 0, 0))
        .setImageExtent(dstImage->GetExtent())
        .setImageSubresource(imageSubresourceLayers);
    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    mCopyCommandBuffer->begin(beginInfo);
    mCopyCommandBuffer->copyBufferToImage(srcBuffer->GetHandle(), dstImage->GetHandle(),
                                          vk::ImageLayout::eTransferDstOptimal, {region});
    mCopyCommandBuffer->end();

    vk::SubmitInfo submitInfo;
    submitInfo.setCommandBuffers(mCopyCommandBuffer.get())
        .setWaitSemaphores(waitSemaphores)
        .setSignalSemaphores(signalSemaphores)
        .setWaitDstStageMask(waitDstStageMask);
    mContext->SubmitToTransferQueue({submitInfo}, fences);
}
vk::Format ImageFactory::GetBestFormat(ImageType type)
{
    switch (type)
    {
    case ImageType::Texture2D:
        return mTexture2DFormat;
    case ImageType::TextureCube:
        return mTextureCubeFormat;
    case ImageType::RenderTarget:
        return mRenderTargetFormat;
    case ImageType::DepthStencil:
        return mDepthStencilFormat;
    case ImageType::Storage:
        return mStorageFormat;
    default:
        mLogger->Error("Invalid image type");
        throw std::invalid_argument("Invalid image type");
    }
}
uint32_t ImageFactory::GetFormatPixelSize(vk::Format format) const
{
    switch (format)
    {
    // 单通道
    // 1字节
    case vk::Format::eR8Unorm:
    case vk::Format::eR8Snorm:
    case vk::Format::eR8Uint:
    case vk::Format::eR8Sint:
    case vk::Format::eR8Srgb:
        return 1;
    case vk::Format::eR16Unorm:
    case vk::Format::eR16Snorm:
    case vk::Format::eR16Uint:
    case vk::Format::eR16Sint:
    case vk::Format::eR16Sfloat:
        return 2;
    case vk::Format::eR32Uint:
    case vk::Format::eR32Sint:
    case vk::Format::eR32Sfloat:
        return 4;
    case vk::Format::eR64Uint:
    case vk::Format::eR64Sint:
    case vk::Format::eR64Sfloat:
        return 8;
    // 双通道
    // 2字节
    case vk::Format::eR8G8Unorm:
    case vk::Format::eR8G8Snorm:
    case vk::Format::eR8G8Uint:
    case vk::Format::eR8G8Sint:
    case vk::Format::eR8G8Srgb:
        return 2;
    case vk::Format::eR16G16Unorm:
    case vk::Format::eR16G16Snorm:
    case vk::Format::eR16G16Uint:
    case vk::Format::eR16G16Sint:
    case vk::Format::eR16G16Sfloat:
        return 4;
    case vk::Format::eR32G32Uint:
    case vk::Format::eR32G32Sint:
    case vk::Format::eR32G32Sfloat:
        return 8;
    case vk::Format::eR64G64Uint:
    case vk::Format::eR64G64Sint:
    case vk::Format::eR64G64Sfloat:
        return 16;
    // 三通道
    case vk::Format::eR8G8B8Unorm:
    case vk::Format::eR8G8B8Snorm:
    case vk::Format::eR8G8B8Uint:
    case vk::Format::eR8G8B8Sint:
    case vk::Format::eR8G8B8Srgb:
        return 3;
    case vk::Format::eR16G16B16Unorm:
    case vk::Format::eR16G16B16Snorm:
    case vk::Format::eR16G16B16Uint:
    case vk::Format::eR16G16B16Sint:
    case vk::Format::eR16G16B16Sfloat:
        return 6;
    case vk::Format::eR32G32B32Uint:
    case vk::Format::eR32G32B32Sint:
    case vk::Format::eR32G32B32Sfloat:
        return 12;
    case vk::Format::eR64G64B64Uint:
    case vk::Format::eR64G64B64Sint:
    case vk::Format::eR64G64B64Sfloat:
        return 24;
    // 四通道
    case vk::Format::eR8G8B8A8Unorm:
    case vk::Format::eR8G8B8A8Snorm:
    case vk::Format::eR8G8B8A8Uint:
    case vk::Format::eR8G8B8A8Sint:
    case vk::Format::eR8G8B8A8Srgb:
    case vk::Format::eB8G8R8A8Unorm:
    case vk::Format::eB8G8R8A8Snorm:
    case vk::Format::eB8G8R8A8Uint:
    case vk::Format::eB8G8R8A8Sint:
    case vk::Format::eB8G8R8A8Srgb:
        return 4;
    case vk::Format::eR16G16B16A16Unorm:
    case vk::Format::eR16G16B16A16Snorm:
    case vk::Format::eR16G16B16A16Uint:
    case vk::Format::eR16G16B16A16Sint:
    case vk::Format::eR16G16B16A16Sfloat:
        return 8;
    case vk::Format::eR32G32B32A32Uint:
    case vk::Format::eR32G32B32A32Sint:
    case vk::Format::eR32G32B32A32Sfloat:
        return 16;
    case vk::Format::eR64G64B64A64Uint:
    case vk::Format::eR64G64B64A64Sint:
    case vk::Format::eR64G64B64A64Sfloat:
        return 32;
    // 其他格式
    case vk::Format::eD16Unorm:
        return 2;
    case vk::Format::eD32Sfloat:
        return 4;
    case vk::Format::eD16UnormS8Uint: // D16(2字节) + S8(1字节) + 1字节填充
        return 4;
    case vk::Format::eD24UnormS8Uint: // D24(3字节) + S8(1字节)
        return 4;
    case vk::Format::eD32SfloatS8Uint: // D32(4字节) + S8(1字节) + 3字节填充
        return 8;

    // 压缩格式（需特别处理）
    case vk::Format::eBc1RgbUnormBlock:
    case vk::Format::eBc1RgbaUnormBlock:
        throw std::runtime_error("Compressed formats require block-based calculation");

    default:
        throw std::runtime_error("Unsupported format");
    }
}
vk::UniqueImageView ImageFactory::CreateImageView(Image *image, vk::ImageAspectFlags aspectMask,
                                                  vk::ComponentMapping components)
{
    if (aspectMask == vk::ImageAspectFlags{})
    {
        switch (image->GetFormat())
        {
        case vk::Format::eD32SfloatS8Uint:
        case vk::Format::eD24UnormS8Uint:
            aspectMask = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
            break;
        case vk::Format::eD32Sfloat:
        case vk::Format::eD16Unorm:
            aspectMask = vk::ImageAspectFlagBits::eDepth;
            break;
        case vk::Format::eR32G32B32A32Sfloat:
        case vk::Format::eR16G16B16A16Sfloat:
        case vk::Format::eR8G8B8A8Srgb:
        case vk::Format::eB8G8R8A8Srgb:
        case vk::Format::eR8G8B8A8Unorm:
        case vk::Format::eB8G8R8A8Unorm:
            aspectMask = vk::ImageAspectFlagBits::eColor;
            break;
        default:
            mLogger->Error("Unsupported image format for aspect mask");
            throw std::runtime_error("Unsupported image format for aspect mask");
        }
    }
    vk::ImageViewType viewType{};
    switch (image->GetImageType())
    {
    case vk::ImageType::e1D:
        viewType = vk::ImageViewType::e1D;
        break;
    case vk::ImageType::e2D:
        viewType = vk::ImageViewType::e2D;
        break;
    case vk::ImageType::e3D:
        viewType = vk::ImageViewType::e3D;
        break;
    default:
        mLogger->Error("Unsupported image type for view type");
        throw std::runtime_error("Unsupported image type for view type");
    }
    vk::ImageSubresourceRange subresourceRange{};
    subresourceRange.setAspectMask(aspectMask)
        .setBaseArrayLayer(0)
        .setLayerCount(image->GetArrayLayers())
        .setBaseMipLevel(0)
        .setLevelCount(image->GetMipLevels());
    vk::ImageViewCreateInfo imageViewCreateInfo{};
    imageViewCreateInfo.setImage(image->GetHandle())
        .setViewType(viewType)
        .setFormat(image->GetFormat())
        .setSubresourceRange(subresourceRange)
        .setComponents(components);
    auto imageView = mContext->GetDevice().createImageViewUnique(imageViewCreateInfo);
    if (!imageView)
    {
        mLogger->Error("Failed to create image view");
    }
    return imageView;
}
void ImageFactory::QueryImageFormat()
{
    vk::FormatProperties formatProperties;
    // Texture2D
    for (auto format : mTexture2DFormats)
    {
        formatProperties = mContext->GetPhysicalDevice().getFormatProperties(format);
        vk::FormatFeatureFlags requiredFeatures =
            vk::FormatFeatureFlagBits::eSampledImage | vk::FormatFeatureFlagBits::eTransferSrc |
            vk::FormatFeatureFlagBits::eTransferDst | vk::FormatFeatureFlagBits::eColorAttachment;
        if (formatProperties.optimalTilingFeatures & requiredFeatures)
        {
            mTexture2DFormat = format;
            break;
        }
    }
    // TextureCube
    for (auto format : mTextureCubeFormats)
    {
        formatProperties = mContext->GetPhysicalDevice().getFormatProperties(format);
        vk::FormatFeatureFlags requiredFeatures =
            vk::FormatFeatureFlagBits::eSampledImage | vk::FormatFeatureFlagBits::eTransferSrc |
            vk::FormatFeatureFlagBits::eTransferDst | vk::FormatFeatureFlagBits::eColorAttachment;
        if (formatProperties.optimalTilingFeatures & requiredFeatures)
        {
            mTextureCubeFormat = format;
            break;
        }
    }
    // RenderTarget
    for (auto format : mRenderTargetFormats)
    {
        formatProperties = mContext->GetPhysicalDevice().getFormatProperties(format);
        vk::FormatFeatureFlags requiredFeatures = vk::FormatFeatureFlagBits::eColorAttachment |
                                                  vk::FormatFeatureFlagBits::eTransferSrc |
                                                  vk::FormatFeatureFlagBits::eSampledImage;
        if (formatProperties.optimalTilingFeatures & requiredFeatures)
        {
            mRenderTargetFormat = format;
            break;
        }
    }
    // DepthStencil
    for (auto format : mDepthStencilCandidintFormats)
    {
        formatProperties = mContext->GetPhysicalDevice().getFormatProperties(format);
        vk::FormatFeatureFlags requiredFeatures = vk::FormatFeatureFlagBits::eDepthStencilAttachment |
                                                  vk::FormatFeatureFlagBits::eTransferSrc |
                                                  vk::FormatFeatureFlagBits::eSampledImage;
        if (formatProperties.optimalTilingFeatures & requiredFeatures)
        {
            mDepthStencilFormat = format;
            break;
        }
    }
    // Storage
    for (auto format : mStorageFormats)
    {
        formatProperties = mContext->GetPhysicalDevice().getFormatProperties(format);
        vk::FormatFeatureFlags requiredFeatures = vk::FormatFeatureFlagBits::eStorageImage |
                                                  vk::FormatFeatureFlagBits::eTransferSrc |
                                                  vk::FormatFeatureFlagBits::eTransferDst;
        if (formatProperties.optimalTilingFeatures & requiredFeatures)
        {
            mStorageFormat = format;
            break;
        }
    }
    // log
    mLogger->Info("Texture2D format: {}", vk::to_string(mTexture2DFormat));
    mLogger->Info("TextureCube format: {}", vk::to_string(mTextureCubeFormat));
    mLogger->Info("RenderTarget format: {}", vk::to_string(mRenderTargetFormat));
    mLogger->Info("DepthStencil format: {}", vk::to_string(mDepthStencilFormat));
    mLogger->Info("Storage format: {}", vk::to_string(mStorageFormat));
}
} // namespace MEngine
