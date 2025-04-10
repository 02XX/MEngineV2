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
    mCommandBuffer = mCommandBufferManager->CreatePrimaryCommandBuffer(CommandBufferType::Transfer);
}
UniqueImage ImageFactory::CreateImage(ImageType type, vk::Extent3D extent, vk::Format format, const void *data,
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
                     vk::ImageUsageFlagBits::eSampled;
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
        .setFormat(format)
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
            // 转换图像布局到eTransferDst
            vk::ImageMemoryBarrier preBarrier{};
            preBarrier.setImage(image->GetHandle())
                .setOldLayout(vk::ImageLayout::eUndefined)
                .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
                .setSrcAccessMask(vk::AccessFlagBits::eNoneKHR)
                .setDstAccessMask(vk::AccessFlagBits::eTransferWrite)
                .setSrcQueueFamilyIndex(mContext->GetQueueFamilyIndicates().transferFamily.value())
                .setDstQueueFamilyIndex(mContext->GetQueueFamilyIndicates().transferFamily.value())
                .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, mipLevels, 0, arrayLayers});
            mCommandBuffer->pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer,
                                            {}, {}, {}, preBarrier);
            // 复制数据到图像
            auto buffer = mBufferFactory->CreateBuffer(BufferType::Staging,
                                                       extent.width * extent.height * GetFormatPixelSize(format), data);
            vk::ImageSubresourceLayers imageSubresourceLayers{};
            imageSubresourceLayers.setAspectMask(vk::ImageAspectFlagBits::eColor)
                .setMipLevel(0)
                .setBaseArrayLayer(0)
                .setLayerCount(arrayLayers);
            CopyBufferToImage(buffer.get(), image.get(), imageSubresourceLayers);
            // 转换图像布局
            vk::ImageMemoryBarrier postBarrier{};
            postBarrier.setImage(image->GetHandle())
                .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
                .setNewLayout(imageLayout)
                .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
                .setDstAccessMask(accessMask)
                .setSrcQueueFamilyIndex(mContext->GetQueueFamilyIndicates().transferFamily.value())
                .setDstQueueFamilyIndex(mContext->GetQueueFamilyIndicates().graphicsFamily.value())
                .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, mipLevels, 0, arrayLayers});
            mCommandBuffer->pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, pipelineStage, {}, {}, {},
                                            postBarrier);
        }
    }
}
void ImageFactory::CopyBufferToImage(Buffer *srcBuffer, Image *dstImage,
                                     vk::ImageSubresourceLayers imageSubresourceLayers)
{
    auto commandBuffer = mCommandBufferManager->CreatePrimaryCommandBuffer(CommandBufferType::Transfer);

    vk::BufferImageCopy region{};
    region
        .setBufferOffset(0)                    // 缓冲区的起始字节偏移
        .setBufferRowLength(0)                 // 缓冲区中每行的像素数（内存布局） 0-和extent一样
        .setBufferImageHeight(0)               // 缓冲区中每列的像素行数（内存布局） 0-和extent一样
        .setImageOffset(vk::Offset3D(0, 0, 0)) // 图像起始坐标（x,y,z）
        .setImageExtent(dstImage->GetExtent()) // 拷贝的区域尺寸
        .setImageSubresource(imageSubresourceLayers);
    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    mCommandBuffer->begin(beginInfo);
    mCommandBuffer->copyBufferToImage(srcBuffer->GetHandle(), dstImage->GetHandle(),
                                      vk::ImageLayout::eTransferDstOptimal, {region});
    mCommandBuffer->end();

    vk::SubmitInfo submitInfo;
    submitInfo.setCommandBuffers(commandBuffer.get());
    mContext->SubmitToTransferQueue({submitInfo}, mFence.get());
    auto result = mContext->GetDevice().waitForFences(mFence.get(), vk::True, 1'000'000'000);
    if (result != vk::Result::eSuccess)
    {
        mLogger->Error("Copy image operation failed");
        throw std::runtime_error("Copy image operation failed");
    }
    mLogger->Debug("Copy image operation success");
}

uint32_t ImageFactory::GetFormatPixelSize(vk::Format format) const
{
    switch (format)
    {
    // 8-bit 单通道格式
    case vk::Format::eR8Unorm:
    case vk::Format::eR8Snorm:
    case vk::Format::eR8Uscaled:
        return 1;

    // 16-bit 双通道格式
    case vk::Format::eR8G8Unorm:
    case vk::Format::eR8G8Snorm:
        return 2;

    // 24-bit RGB 无压缩格式
    case vk::Format::eR8G8B8Unorm:
    case vk::Format::eB8G8R8Srgb:
        return 3;

    // 32-bit 格式（含四通道/深度）
    case vk::Format::eR8G8B8A8Unorm:
    case vk::Format::eB8G8R8A8Srgb:
    case vk::Format::eD32Sfloat:
        return 4;

    // 16-bit 浮点格式
    case vk::Format::eR16Sfloat:
        return 2;
    case vk::Format::eR16G16Sfloat:
        return 4;

    // 64-bit 格式
    case vk::Format::eR16G16B16A16Sfloat:
        return 8;

    // 128-bit 浮点格式（用户已实现）
    case vk::Format::eR32G32B32A32Sfloat:
        return 16;

    // 特殊格式处理
    case vk::Format::eD24UnormS8Uint: // 深度+模板（24+8 bits）
        return 4;

    // 压缩格式（需特别处理）
    case vk::Format::eBc1RgbUnormBlock:
    case vk::Format::eBc1RgbaUnormBlock:
        throw std::runtime_error("Compressed formats require block-based calculation");

    default:
        throw std::runtime_error("Unsupported format");
    }
}
} // namespace MEngine
