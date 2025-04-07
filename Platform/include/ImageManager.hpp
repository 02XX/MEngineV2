#pragma once
#include "Buffer.hpp"
#include "BufferManager.hpp"
#include "CommandBuffeManager.hpp"
#include "Image.hpp"
#include "MEngine.hpp"
#include "NoCopyable.hpp"
#include "SyncPrimitiveManager.hpp"
#include <memory>

namespace MEngine
{
class ImageManager final : public NoCopyable
{
  private:
    // DI
    std::shared_ptr<Context> mContext;
    std::shared_ptr<ILogger> mLogger;

    std::shared_ptr<CommandBufferManager> mCommandBufferManager;
    std::shared_ptr<SyncPrimitiveManager> mSyncPrimitiveManager;
    std::shared_ptr<BufferManager> mBufferManager;

  public:
    ImageManager(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                 std::shared_ptr<CommandBufferManager> commandBufferManager = nullptr,
                 std::shared_ptr<SyncPrimitiveManager> syncPrimitiveManager = nullptr,
                 std::shared_ptr<BufferManager> bufferManager = nullptr);

    /**
     * @brief Create a 2D texture (GPU Only)
     * @param width Texture width
     * @param height Texture height
     * @param format Pixel format
     * @param mipLevels Mipmap levels
     * @param data Initial data (optional)
     */
    UniqueImage CreateUniqueTexture2D(vk::Extent2D extent, vk::Format format = vk::Format::eR8G8B8A8Srgb,
                                      uint32_t mipLevels = 1, const void *data = nullptr);

    /**
     * @brief Create a render target (GPU Only)
     * @param extent Image extent
     * @param format Pixel format
     * @param usage Additional usage flags
     */
    UniqueImage CreateUniqueRenderTarget(vk::Extent2D extent, vk::Format format = vk::Format::eR8G8B8A8Srgb,
                                         vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment,
                                         uint32_t mipLevels = 1,
                                         vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1);

    /**
     * @brief Create a depth/stencil buffer (GPU Only)
     * @param extent Buffer extent
     * @param format Depth format
     */
    UniqueImage CreateUniqueDepthStencil(vk::Extent2D extent, vk::Format format = vk::Format::eD32SfloatS8Uint,
                                         uint32_t mipLevels = 1,
                                         vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1);

    /**
     * @brief Create a storage image (GPU Only)
     * @param width Image width
     * @param height Image height
     * @param format Storage format
     */
    UniqueImage CreateUniqueStorageImage(vk::Extent2D extent, vk::Format format = vk::Format::eR8G8B8A8Srgb,
                                         uint32_t mipLevels = 1,
                                         vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1);
    /**
     * @brief Copy buffer to image
     * @param srcBuffer Source buffer
     * @param dstImage Destination image
     * @param extent Image extent
     * @param layerCount Image layer count
     */
    void CopyBufferToImage(vk::Buffer srcBuffer, vk::Image dstImage, vk::Extent2D extent,
                           vk::ImageSubresourceLayers imageSubresourceLayers = {vk::ImageAspectFlagBits::eColor, 0, 0,
                                                                                1});

    /**
     * @brief Transition image layout
     * @param image Target image
     * @param oldLayout Old layout
     * @param newLayout New layout
     * @param aspectMask Image aspect (color/depth, etc.)
     */
    void TransitionLayout(vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
                          vk::ImageSubresourceRange subresourceRange);

    /**
     * @brief Get the Format Pixel Size object
     *
     * @param format image format
     * @return uint32_t
     */
    uint32_t GetFormatPixelSize(vk::Format format) const;
    /**
     * @brief Get the Access Masks For Layout object
     *
     * @param oldLayout
     * @param newLayout
     * @return std::pair<vk::AccessFlags, vk::AccessFlags>
     */
    std::pair<vk::AccessFlags, vk::AccessFlags> GetAccessMasksForLayout(vk::ImageLayout oldLayout,
                                                                        vk::ImageLayout newLayout);
    /**
     * @brief Get the Pipeline Stages For Layout object
     *
     * @param oldLayout
     * @param newLayout
     * @return std::pair<vk::PipelineStageFlags, vk::PipelineStageFlags>
     */
    std::pair<vk::PipelineStageFlags, vk::PipelineStageFlags> GetPipelineStagesForLayout(vk::ImageLayout oldLayout,
                                                                                         vk::ImageLayout newLayout);
    /**
     * @brief Create a Image View object
     *
     * @param image
     * @param format
     * @param components
     * @param subresourceRange
     * @return vk::UniqueImageView
     */
    vk::UniqueImageView CreateImageView(vk::Image image, vk::Format format, vk::ComponentMapping components = {},
                                        vk::ImageSubresourceRange subresourceRange = {vk::ImageAspectFlagBits::eColor,
                                                                                      0, 1, 0, 1});
};
} // namespace MEngine
