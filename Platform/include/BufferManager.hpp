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
class MENGINE_API BufferManager final : public NoCopyable
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
    UniqueBuffer CreateUniqueVertexBuffer(vk::DeviceSize size, const void *data = nullptr);
    SharedBuffer CreateSharedVertexBuffer(vk::DeviceSize size, const void *data = nullptr);

    /**
     * @brief Create a Index Buffer object, GPU Only
     *
     * @param size buffer size
     * @param data buffer data
     * @return UniqueBuffer
     */
    UniqueBuffer CreateUniqueIndexBuffer(vk::DeviceSize size, const void *data = nullptr);
    SharedBuffer CreateSharedIndexBuffer(vk::DeviceSize size, const void *data = nullptr);

    /**
     * @brief Create a Uniform Buffer object, CPU And GPU
     *
     * @param size buffer size
     * @return UniqueBuffer
     */
    UniqueBuffer CreateUniqueUniformBuffer(vk::DeviceSize size);
    SharedBuffer CreateSharedUniformBuffer(vk::DeviceSize size);

    /**
     * @brief Create a Staging Buffer object, CPU Only
     *
     * @param size buffer size
     * @param data buffer data
     * @return UniqueBuffer
     */
    UniqueBuffer CreateUniqueStagingBuffer(vk::DeviceSize size, const void *data = nullptr);
    SharedBuffer CreateSharedStagingBuffer(vk::DeviceSize size, const void *data = nullptr);
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
