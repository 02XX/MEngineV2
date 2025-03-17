#include "BufferFactory.hpp"
#include <vulkan/vulkan_enums.hpp>

namespace MEngine
{
UniqueBuffer BufferFactory::CreateVertexBuffer(vk::DeviceSize size, const void *data)
{
}

UniqueBuffer BufferFactory::CreateIndexBuffer(vk::DeviceSize size, const void *data)
{
}

UniqueBuffer BufferFactory::CreateUniformBuffer(vk::DeviceSize size)
{
}

UniqueBuffer BufferFactory::CreateStagingBuffer(vk::DeviceSize size, const void *data)
{
    constexpr auto flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

    auto staging = std::make_unique<Buffer>(size,
                                            vk::BufferUsageFlagBits::eTransferSrc, // 用途：传输源
                                            VMA_MEMORY_USAGE_CPU_ONLY,             // 内存类型：CPU可见且相干
                                            flags);
    if (data)
    {
        void *mapped = staging->GetAllocationInfo().pMappedData;
        std::memcpy(
            mapped, data,
            size); // 这里使用size而不是staging->GetAllocationInfo().size，是因为实际分配的内存可能比请求的内存大（内存对齐的要求）
    }
    return staging;
}
} // namespace MEngine