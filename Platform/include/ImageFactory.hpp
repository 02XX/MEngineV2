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
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan_enums.hpp>

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
    vk::UniqueCommandBuffer mCopyCommandBuffer;
    vk::UniqueCommandBuffer mPreTransitionCommandBuffer;
    vk::UniqueCommandBuffer mPostTransitionCommandBuffer;
    vk::UniqueFence mFence;
    vk::UniqueSemaphore mPreTransitionDone;
    vk::UniqueSemaphore mPostTransitionDone;
    vk::UniqueSemaphore mCopyDone;

    std::vector<vk::Format> mTexture2DFormats;
    std::vector<vk::Format> mTextureCubeFormats;
    std::vector<vk::Format> mRenderTargetFormats;
    std::vector<vk::Format> mDepthStencilCandidintFormats;
    std::vector<vk::Format> mStorageFormats;

    vk::Format mTexture2DFormat;
    vk::Format mTextureCubeFormat;
    vk::Format mRenderTargetFormat;
    vk::Format mDepthStencilFormat;
    vk::Format mStorageFormat;

  public:
    ImageFactory(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                 std::shared_ptr<CommandBufferManager> commandBufferManager,
                 std::shared_ptr<SyncPrimitiveManager> syncPrimitiveManager,
                 std::shared_ptr<BufferFactory> bufferFactory);

    UniqueImage CreateImage(ImageType type, vk::Extent3D extent, uint32_t mipLevels = 1,
                            vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1);

    UniqueImage CreateImage(ImageType type, vk::Extent3D extent, vk::DeviceSize size, const void *data,
                            uint32_t mipLevels = 1, vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1);
    vk::UniqueImageView CreateImageView(Image *image, vk::ImageAspectFlags aspectMask = {},
                                        vk::ComponentMapping components = {});
    void TransitionImageLayout(Image *image, vk::ImageLayout newLayout, vk::PipelineStageFlagBits srcStage,
                               vk::PipelineStageFlagBits dstStage, vk::AccessFlags srcAccessMask,
                               vk::AccessFlags dstAccessMask, vk::ImageSubresourceRange subresourceRange);
    void CopyBufferToImage(Buffer *srcBuffer, Image *dstImage, vk::ImageSubresourceLayers imageSubresourceLayers);

  private:
    void QueryImageFormat();
    vk::Format GetBestFormat(ImageType type);
    uint32_t GetFormatPixelSize(vk::Format format) const;

  public:
    inline vk::Format GetTexture2DFormat() const
    {
        return mTexture2DFormat;
    }
    inline vk::Format GetTextureCubeFormat() const
    {
        return mTextureCubeFormat;
    }
    inline vk::Format GetRenderTargetFormat() const
    {
        return mRenderTargetFormat;
    }
    inline vk::Format GetDepthStencilFormat() const
    {
        return mDepthStencilFormat;
    }
    inline vk::Format GetStorageFormat() const
    {
        return mStorageFormat;
    }
};
} // namespace MEngine
