#pragma once
#include "Buffer.hpp"
#include "MEngine.hpp"
#include "VMA.hpp"
#include <vulkan/vulkan.hpp>
namespace MEngine
{
class MENGINE_API BufferFactory final
{
  public:
    BufferFactory() = default;
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
};
} // namespace MEngine
