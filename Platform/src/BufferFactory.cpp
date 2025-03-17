#include "BufferFactory.hpp"

namespace MEngine
{
BufferFactory::BufferFactory()
{
    mCommandBufferFactory =
        std::make_unique<CommandBufferFactory>(Context::Instance().GetQueueFamilyIndicates().transferFamily.value());
    mSyncPrimitiveFactory = std::make_unique<SyncPrimitiveFactory>();
}

UniqueBuffer BufferFactory::CreateVertexBuffer(vk::DeviceSize size, const void *data)
{
    constexpr auto usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst;
    auto vertexBuffer = std::make_unique<Buffer>(size, usage, VMA_MEMORY_USAGE_GPU_ONLY);
    if (data)
    {
        auto staging = CreateStagingBuffer(size, data);
        CopyBuffer(staging->GetBuffer(), vertexBuffer->GetBuffer(), size);
    }
    LogD("Vertex buffer created and data copied successfully");
    return vertexBuffer;
}

UniqueBuffer BufferFactory::CreateIndexBuffer(vk::DeviceSize size, const void *data)
{
    constexpr auto usage = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
    auto indexBuffer = std::make_unique<Buffer>(size, usage, VMA_MEMORY_USAGE_GPU_ONLY);
    if (data)
    {
        auto staging = CreateStagingBuffer(size, data);
        CopyBuffer(staging->GetBuffer(), indexBuffer->GetBuffer(), size);
    }
    LogD("Index buffer created and data copied successfully");
    return indexBuffer;
}

UniqueBuffer BufferFactory::CreateUniformBuffer(vk::DeviceSize size)
{
    constexpr auto flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
    auto buffer =
        std::make_unique<Buffer>(size, vk::BufferUsageFlagBits::eUniformBuffer, VMA_MEMORY_USAGE_CPU_TO_GPU, flags);
    LogD("Uniform buffer created and data copied successfully");
    return buffer;
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
    LogD("Staging buffer created and data copied successfully");
    return staging;
}

void BufferFactory::CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size)
{
    auto &context = Context::Instance();
    auto commandBuffer = mCommandBufferFactory->CreatePrimaryCommandBuffer();
    auto fence = mSyncPrimitiveFactory->CreateFence();
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