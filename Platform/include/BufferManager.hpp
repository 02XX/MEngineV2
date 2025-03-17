#pragma once
#include "Buffer.hpp"
#include "CommandBuffeManager.hpp"
#include "MEngine.hpp"
#include "SyncPrimitiveManager.hpp"
#include "VMA.hpp"
#include <memory>
#include <vulkan/vulkan.hpp>

namespace MEngine
{
class MENGINE_API BufferManager final
{
  private:
    std::unique_ptr<CommandBufferManager> mCommandBufferManager;
    std::unique_ptr<SyncPrimitiveManager> mSyncPrimitiveManager;

  public:
    BufferManager();
    /**
     * @brief Create a Vertex Buffer object, GPU Only
     *
     * @param size buffer size
     * @param data buffer data
     * @return UniqueBuffer
     */
    UniqueBuffer CreateVertexBuffer(vk::DeviceSize size, const void *data = nullptr);

    /**
     * @brief Create a Index Buffer object, GPU Only
     *
     * @param size buffer size
     * @param data buffer data
     * @return UniqueBuffer
     */
    UniqueBuffer CreateIndexBuffer(vk::DeviceSize size, const void *data = nullptr);

    /**
     * @brief Create a Uniform Buffer object, CPU And GPU
     *
     * @param size buffer size
     * @return UniqueBuffer
     */
    UniqueBuffer CreateUniformBuffer(vk::DeviceSize size);

    /**
     * @brief Create a Staging Buffer object, CPU Only
     *
     * @param size buffer size
     * @param data buffer data
     * @return UniqueBuffer
     */
    UniqueBuffer CreateStagingBuffer(vk::DeviceSize size, const void *data = nullptr);
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
