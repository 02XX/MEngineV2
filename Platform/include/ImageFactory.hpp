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
    const std::vector<vk::Format> mTexture2DFormats = {
        vk::Format::eR32G32B32A32Sfloat, // HDR
        vk::Format::eR16G16B16A16Sfloat, // HDR
        vk::Format::eR8G8B8A8Srgb,       // HDR
        vk::Format::eB8G8R8A8Srgb,       vk::Format::eR8G8B8A8Unorm,
    };
    const std::vector<vk::Format> mTextureCubeFormats = {
        vk::Format::eR32G32B32A32Sfloat, // HDR
        vk::Format::eR16G16B16A16Sfloat, // HDR
        vk::Format::eR8G8B8A8Srgb,       // HDR
        vk::Format::eB8G8R8A8Srgb,       vk::Format::eR8G8B8A8Unorm,
    };
    const std::vector<vk::Format> mRenderTargetFormats = {
        vk::Format::eR32G32B32A32Sfloat, // HDR
        vk::Format::eR16G16B16A16Sfloat, // HDR
        vk::Format::eR8G8B8A8Srgb,       // HDR
        vk::Format::eB8G8R8A8Srgb,       vk::Format::eR8G8B8A8Unorm,
    };
    const std::vector<vk::Format> mDepthStencilCandidintFormats = {
        vk::Format::eD32SfloatS8Uint,
        vk::Format::eD24UnormS8Uint,
    };
    const std::vector<vk::Format> mStorageFormats = {
        vk::Format::eR32G32B32A32Sfloat, // HDR
        vk::Format::eR16G16B16A16Sfloat, // HDR
        vk::Format::eR8G8B8A8Srgb,       // HDR
        vk::Format::eB8G8R8A8Srgb,       vk::Format::eR8G8B8A8Unorm,
    };

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

    UniqueImage CreateImage(ImageType type, vk::Extent3D extent, const void *data, uint32_t mipLevels = 1,
                            vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1);
    vk::UniqueImageView CreateImageView(Image *image, vk::ImageAspectFlags aspectMask = {},
                                        vk::ComponentMapping components = {});

  private:
    void QueryImageFormat();
    vk::Format GetBestFormat(ImageType type);
    uint32_t GetFormatPixelSize(vk::Format format) const;
    void CopyBufferToImage(Buffer *srcBuffer, Image *dstImage, vk::ImageSubresourceLayers imageSubresourceLayers);
};
} // namespace MEngine
