#include "CommandBuffeManager.hpp"

namespace MEngine
{
CommandBufferManager::CommandBufferManager(uint32_t queueFamilyIndex) : mQueueFamilyIndex(queueFamilyIndex)
{
    auto &context = Context::Instance();
    vk::CommandPoolCreateInfo commandPoolCreateInfo;
    commandPoolCreateInfo.setQueueFamilyIndex(queueFamilyIndex)
        .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
    mCommandPool = context.GetDevice().createCommandPoolUnique(commandPoolCreateInfo);
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
    auto &context = Context::Instance();
    vk::CommandBufferAllocateInfo commandBufferAllocateInfo;
    commandBufferAllocateInfo.setLevel(vk::CommandBufferLevel::ePrimary)
        .setCommandPool(mCommandPool.get())
        .setCommandBufferCount(count);
    auto buffers = context.GetDevice().allocateCommandBuffersUnique(commandBufferAllocateInfo);
    LogD("Allocated {} primary command buffers.", std::to_string(count));
    return buffers;
}

std::vector<vk::UniqueCommandBuffer> CommandBufferManager::CreateSecondaryCommandBuffers(uint32_t count)
{
    auto &context = Context::Instance();
    vk::CommandBufferAllocateInfo commandBufferAllocateInfo;
    commandBufferAllocateInfo.setLevel(vk::CommandBufferLevel::eSecondary)
        .setCommandPool(mCommandPool.get())
        .setCommandBufferCount(count);

    auto buffers = context.GetDevice().allocateCommandBuffersUnique(commandBufferAllocateInfo);
    LogD("Allocated {} primary command buffers.", std::to_string(count));
    return buffers;
}
} // namespace MEngine