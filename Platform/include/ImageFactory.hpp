#pragma once
#include "Buffer.hpp"
#include "BufferFactory.hpp"
#include "CommandBuffeManager.hpp"
#include "Image.hpp"
#include "MEngine.hpp"
#include "NoCopyable.hpp"
#include "SyncPrimitiveManager.hpp"
#include "VMA.hpp"
#include <memory>

namespace MEngine
{
enum class ImageType
{
    Texture2D,    // 普通2D纹理
    TextureCube,  // 立方体贴图
    RenderTarget, // 渲染目标
    DepthStencil, // 深度/模板附件
    Storage       // 存储图像
};
class ImageFactory final : public NoCopyable
{
  private:
    // DI
    std::shared_ptr<Context> mContext;
    std::shared_ptr<ILogger> mLogger;

    std::shared_ptr<CommandBufferManager> mCommandBufferManager;
    std::shared_ptr<SyncPrimitiveManager> mSyncPrimitiveManager;
    std::shared_ptr<BufferFactory> mBufferFactory;

  private:
    vk::UniqueCommandBuffer mCommandBuffer;
    vk::UniqueFence mFence;

  public:
    ImageFactory(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                 std::shared_ptr<CommandBufferManager> commandBufferManager,
                 std::shared_ptr<SyncPrimitiveManager> syncPrimitiveManager,
                 std::shared_ptr<BufferFactory> bufferFactory);

    UniqueImage CreateImage(ImageType type, vk::Extent3D extent, vk::Format format, uint32_t mipLevels = 1,
                            vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1);

    UniqueImage CreateImage(ImageType type, vk::Extent3D extent, vk::Format format, const void *data,
                            uint32_t mipLevels = 1, vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1);

  private:
    uint32_t GetFormatPixelSize(vk::Format format) const;
    void CopyBufferToImage(Buffer *srcBuffer, Image *dstImage, vk::ImageSubresourceLayers imageSubresourceLayers);
};
} // namespace MEngine
