#include "CommandBuffeManager.hpp"

namespace MEngine
{
CommandBufferManager::CommandBufferManager(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                                           uint32_t queueFamilyIndex)
    : mQueueFamilyIndex(queueFamilyIndex), mContext(context), mLogger(logger)
{
    vk::CommandPoolCreateInfo commandPoolCreateInfo;
    commandPoolCreateInfo.setQueueFamilyIndex(queueFamilyIndex)
        .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
    mCommandPool = mContext->GetDevice().createCommandPoolUnique(commandPoolCreateInfo);
}

vk::UniqueCommandBuffer CommandBufferManager::CreatePrimaryCommandBuffer()
{
    return std::move(CreatePrimaryCommandBuffers(1)[0]);
}

vk::UniqueCommandBuffer CommandBufferManager::CreateSecondaryCommandBuffer()
{
    return std::move(CreateSecondaryCommandBuffers(1)[0]);
}

std::vector<vk::UniqueCommandBuffer> CommandBufferManager::CreatePrimaryCommandBuffers(uint32_t count)
{
    vk::CommandBufferAllocateInfo commandBufferAllocateInfo;
    commandBufferAllocateInfo.setLevel(vk::CommandBufferLevel::ePrimary)
        .setCommandPool(mCommandPool.get())
        .setCommandBufferCount(count);
    auto buffers = mContext->GetDevice().allocateCommandBuffersUnique(commandBufferAllocateInfo);
    mLogger->Debug("Allocated {} primary command buffers.", std::to_string(count));
    return buffers;
}

std::vector<vk::UniqueCommandBuffer> CommandBufferManager::CreateSecondaryCommandBuffers(uint32_t count)
{
    vk::CommandBufferAllocateInfo commandBufferAllocateInfo;
    commandBufferAllocateInfo.setLevel(vk::CommandBufferLevel::eSecondary)
        .setCommandPool(mCommandPool.get())
        .setCommandBufferCount(count);

    auto buffers = mContext->GetDevice().allocateCommandBuffersUnique(commandBufferAllocateInfo);
    mLogger->Debug("Allocated {} primary command buffers.", std::to_string(count));
    return buffers;
}
} // namespace MEngine