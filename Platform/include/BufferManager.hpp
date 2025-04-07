#pragma once
#include "Buffer.hpp"
#include "CommandBuffeManager.hpp"
#include "MEngine.hpp"
#include "NoCopyable.hpp"
#include "SyncPrimitiveManager.hpp"
#include "VMA.hpp"
#include <memory>
#include <vulkan/vulkan.hpp>

namespace MEngine
{
class BufferManager final : public NoCopyable
{
  private:
    // DI
    std::shared_ptr<Context> mContext;
    std::shared_ptr<ILogger> mLogger;

    std::shared_ptr<CommandBufferManager> mCommandBufferManager;
    std::shared_ptr<SyncPrimitiveManager> mSyncPrimitiveManager;

  public:
    BufferManager(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                  std::shared_ptr<CommandBufferManager> commandBufferManager = nullptr,
                  std::shared_ptr<SyncPrimitiveManager> syncPrimitiveManager = nullptr);
    /**
     * @brief Create a Vertex Buffer object, GPU Only
     *
     * @param size buffer size
     * @param data buffer data
     * @return UniqueBuffer
     */
    UniqueBuffer CreateUniqueVertexBuffer(vk::DeviceSize size, const void *data = nullptr);

    /**
     * @brief Create a Index Buffer object, GPU Only
     *
     * @param size buffer size
     * @param data buffer data
     * @return UniqueBuffer
     */
    UniqueBuffer CreateUniqueIndexBuffer(vk::DeviceSize size, const void *data = nullptr);

    /**
     * @brief Create a Uniform Buffer object, CPU And GPU
     *
     * @param size buffer size
     * @return UniqueBuffer
     */
    UniqueBuffer CreateUniqueUniformBuffer(vk::DeviceSize size);

    /**
     * @brief Create a Staging Buffer object, CPU Only
     *
     * @param size buffer size
     * @param data buffer data
     * @return UniqueBuffer
     */
    UniqueBuffer CreateUniqueStagingBuffer(vk::DeviceSize size, const void *data = nullptr);
    /**
     * @brief Copy Buffer from srcBuffer to dstBuffer
     *
     * @param srcBuffer source buffer
     * @param dstBuffer destination buffer
     * @param size size
     */
    void CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);
};
} // namespace MEngine
