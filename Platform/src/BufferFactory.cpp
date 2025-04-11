#include "BufferFactory.hpp"

namespace MEngine
{
BufferFactory::BufferFactory(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                             std::shared_ptr<CommandBufferManager> commandBufferManager,
                             std::shared_ptr<SyncPrimitiveManager> syncPrimitiveManager)
    : mContext(context), mLogger(logger), mCommandBufferManager(commandBufferManager),
      mSyncPrimitiveManager(syncPrimitiveManager)
{
    mCommandBuffer = mCommandBufferManager->CreatePrimaryCommandBuffer(CommandBufferType::Transfer);
    mFence = mSyncPrimitiveManager->CreateFence();
}
UniqueBuffer BufferFactory::CreateBuffer(BufferType type, vk::DeviceSize size, const void *data)
{
    VmaMemoryUsage memoryUsage{};
    vk::BufferUsageFlags bufferUsage{};
    VmaAllocationCreateFlags createflags{};
    switch (type)
    {
    case BufferType::Vertex:
        memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
        bufferUsage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst;
        break;
    case BufferType::Index:
        memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
        bufferUsage = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
        break;
    case BufferType::Uniform:
        memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        bufferUsage = vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst;
        createflags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
        break;
    case BufferType::Staging:
        memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;
        bufferUsage = vk::BufferUsageFlagBits::eTransferSrc;
        createflags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        break;
    case BufferType::Storage:
        memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
        bufferUsage = vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst;
        break;
    default:
        mLogger->Error("Invalid buffer type");
        throw std::invalid_argument("Invalid buffer type");
    }
    auto buffer = std::make_unique<Buffer>(mContext, size, bufferUsage, memoryUsage, createflags);
    if (data)
    {
        if (type == BufferType::Staging)
        {
            void *mapped = buffer->GetAllocationInfo().pMappedData;
            std::memcpy(mapped, data, size);
        }
        else
        {
            memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
            bufferUsage = vk::BufferUsageFlagBits::eTransferSrc;
            createflags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
            auto staging = std::make_unique<Buffer>(mContext, size, bufferUsage, memoryUsage, createflags);
            void *mapped = staging->GetAllocationInfo().pMappedData;
            std::memcpy(mapped, data, size);
            mContext->GetDevice().resetFences({mFence.get()});
            CopyBuffer(staging.get(), buffer.get());
            auto result = mContext->GetDevice().waitForFences(mFence.get(), vk::True, 1'000'000'000);
            if (result != vk::Result::eSuccess)
            {
                mLogger->Error("Copy buffer operation failed");
                throw std::runtime_error("Copy buffer operation failed");
            }
        }
    }
    return buffer;
}
void BufferFactory::CopyBuffer(Buffer *src, Buffer *dst)
{
    mCommandBuffer->reset();
    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    mCommandBuffer->begin(beginInfo);
    {
        vk::BufferCopy copyRegion{};
        copyRegion.setSize(src->GetSize()).setDstOffset(0).setSrcOffset(0);
        mCommandBuffer->copyBuffer(src->GetHandle(), dst->GetHandle(), copyRegion);
    }
    mCommandBuffer->end();
    vk::SubmitInfo submitInfo{};
    submitInfo.setCommandBuffers(mCommandBuffer.get());
    mContext->SubmitToTransferQueue({submitInfo}, mFence.get());
    auto result = mContext->GetDevice().waitForFences(mFence.get(), vk::True, 1'000'000'000);
    if (result != vk::Result::eSuccess)
    {
        mLogger->Error("Copy buffer operation failed");
        throw std::runtime_error("Copy buffer operation failed");
    }
}
} // namespace MEngine