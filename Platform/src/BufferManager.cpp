#include "BufferManager.hpp"
#include "Buffer.hpp"

namespace MEngine
{
BufferManager::BufferManager()
{
    mCommandBufferManager =
        std::make_unique<CommandBufferManager>(Context::Instance().GetQueueFamilyIndicates().transferFamily.value());
    mSyncPrimitiveManager = std::make_unique<SyncPrimitiveManager>();
}

UniqueBuffer BufferManager::CreateUniqueVertexBuffer(vk::DeviceSize size, const void *data)
{
    constexpr auto usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst;
    auto vertexBuffer = std::make_unique<Buffer>(size, usage, VMA_MEMORY_USAGE_GPU_ONLY);
    if (data)
    {
        auto staging = CreateUniqueStagingBuffer(size, data);
        CopyBuffer(staging->GetBuffer(), vertexBuffer->GetBuffer(), size);
    }
    LogD("Vertex buffer created and data copied successfully");
    return vertexBuffer;
}
SharedBuffer BufferManager::CreateSharedVertexBuffer(vk::DeviceSize size, const void *data)
{
    auto uniqueVertexBuffer = CreateUniqueVertexBuffer(size, data);
    return std::move(uniqueVertexBuffer);
}

UniqueBuffer BufferManager::CreateUniqueIndexBuffer(vk::DeviceSize size, const void *data)
{
    constexpr auto usage = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
    auto indexBuffer = std::make_unique<Buffer>(size, usage, VMA_MEMORY_USAGE_GPU_ONLY);
    if (data)
    {
        auto staging = CreateUniqueStagingBuffer(size, data);
        CopyBuffer(staging->GetBuffer(), indexBuffer->GetBuffer(), size);
    }
    LogD("Index buffer created and data copied successfully");
    return indexBuffer;
}
SharedBuffer BufferManager::CreateSharedIndexBuffer(vk::DeviceSize size, const void *data)
{
    auto uniqueIndexBuffer = CreateUniqueIndexBuffer(size, data);
    return std::move(uniqueIndexBuffer);
}

UniqueBuffer BufferManager::CreateUniqueUniformBuffer(vk::DeviceSize size)
{
    constexpr auto flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
    auto buffer =
        std::make_unique<Buffer>(size, vk::BufferUsageFlagBits::eUniformBuffer, VMA_MEMORY_USAGE_CPU_TO_GPU, flags);
    LogD("Uniform buffer created and data copied successfully");
    return buffer;
}
SharedBuffer BufferManager::CreateSharedUniformBuffer(vk::DeviceSize size)
{
    auto uniqueBuffer = CreateUniqueUniformBuffer(size);
    return std::move(uniqueBuffer);
}

UniqueBuffer BufferManager::CreateUniqueStagingBuffer(vk::DeviceSize size, const void *data)
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
    LogD("Staging buffer created and data copied successfully");
    return staging;
}
SharedBuffer BufferManager::CreateSharedStagingBuffer(vk::DeviceSize size, const void *data)
{
    auto uniqueStaging = CreateUniqueStagingBuffer(size, data);
    return std::move(uniqueStaging);
}

void BufferManager::CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size)
{
    auto &context = Context::Instance();
    auto commandBuffer = mCommandBufferManager->CreatePrimaryCommandBuffer();
    auto fence = mSyncPrimitiveManager->CreateFence();
    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    commandBuffer->begin(beginInfo);
    {
        vk::BufferCopy copyRegion{};
        copyRegion.setSize(size).setDstOffset(0).setSrcOffset(0);
        commandBuffer->copyBuffer(srcBuffer, dstBuffer, copyRegion);
    }
    commandBuffer->end();
    vk::SubmitInfo submitInfo{};
    submitInfo.setCommandBuffers(commandBuffer.get());
    context.SubmitToTransferQueue({submitInfo}, fence);

    auto result = context.GetDevice()->waitForFences(fence.get(), vk::True, 1'000'000'000);
    if (result != vk::Result::eSuccess)
    {
        LogE("Copy buffer operation failed");
        throw std::runtime_error("Copy buffer operation failed");
    }
    LogD("Copy buffer operation success");
}
} // namespace MEngine