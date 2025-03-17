#include "CommandBuffeFactory.hpp"

namespace MEngine
{
CommandBufferFactory::CommandBufferFactory(uint32_t queueFamilyIndex) : mQueueFamilyIndex(queueFamilyIndex)
{
    auto &context = Context::Instance();
    vk::CommandPoolCreateInfo commandPoolCreateInfo;
    commandPoolCreateInfo.setQueueFamilyIndex(queueFamilyIndex)
        .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
    mCommandPool = context.GetDevice()->createCommandPoolUnique(commandPoolCreateInfo);
}

vk::UniqueCommandBuffer CommandBufferFactory::CreatePrimaryCommandBuffer()
{
    return std::move(CreatePrimaryCommandBuffers(1)[0]);
}

vk::UniqueCommandBuffer CommandBufferFactory::CreateSecondaryCommandBuffer()
{
    return std::move(CreateSecondaryCommandBuffers(1)[0]);
}

std::vector<vk::UniqueCommandBuffer> CommandBufferFactory::CreatePrimaryCommandBuffers(uint32_t count)
{
    auto &context = Context::Instance();
    vk::CommandBufferAllocateInfo commandBufferAllocateInfo;
    commandBufferAllocateInfo.setLevel(vk::CommandBufferLevel::ePrimary)
        .setCommandPool(mCommandPool.get())
        .setCommandBufferCount(count);
    auto buffers = context.GetDevice()->allocateCommandBuffersUnique(commandBufferAllocateInfo);
    LogD("Allocated {} primary command buffers.", std::to_string(count));
    return buffers;
}

std::vector<vk::UniqueCommandBuffer> CommandBufferFactory::CreateSecondaryCommandBuffers(uint32_t count)
{
    auto &context = Context::Instance();
    vk::CommandBufferAllocateInfo commandBufferAllocateInfo;
    commandBufferAllocateInfo.setLevel(vk::CommandBufferLevel::eSecondary)
        .setCommandPool(mCommandPool.get())
        .setCommandBufferCount(count);

    auto buffers = context.GetDevice()->allocateCommandBuffersUnique(commandBufferAllocateInfo);
    LogD("Allocated {} primary command buffers.", std::to_string(count));
    return buffers;
}
} // namespace MEngine