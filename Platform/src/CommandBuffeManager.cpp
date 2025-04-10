#include "CommandBuffeManager.hpp"

namespace MEngine
{
CommandBufferManager::CommandBufferManager(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context)
    : mContext(context), mLogger(logger)
{
    auto graphicQueueFamilyIndex = mContext->GetQueueFamilyIndicates().graphicsFamily.value();
    auto transferQueueFamilyIndex = mContext->GetQueueFamilyIndicates().transferFamily.value();
    auto presentQueueFamilyIndex = mContext->GetQueueFamilyIndicates().presentFamily.value();
    vk::CommandPoolCreateInfo graphicCommandPoolCreateInfo{};
    vk::CommandPoolCreateInfo transferCommandPoolCreateInfo{};
    vk::CommandPoolCreateInfo presentCommandPoolCreateInfo{};
    graphicCommandPoolCreateInfo.setQueueFamilyIndex(graphicQueueFamilyIndex)
        .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
    transferCommandPoolCreateInfo.setQueueFamilyIndex(transferQueueFamilyIndex)
        .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
    presentCommandPoolCreateInfo.setQueueFamilyIndex(presentQueueFamilyIndex)
        .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
    // Create Command Pool
    mCommandPools[CommandBufferType::Graphic] =
        mContext->GetDevice().createCommandPoolUnique(graphicCommandPoolCreateInfo);
    mCommandPools[CommandBufferType::Transfer] =
        mContext->GetDevice().createCommandPoolUnique(transferCommandPoolCreateInfo);
    mCommandPools[CommandBufferType::Present] =
        mContext->GetDevice().createCommandPoolUnique(presentCommandPoolCreateInfo);
}

vk::CommandBuffer CommandBufferManager::CreatePrimaryCommandBuffer(CommandBufferType type)
{
    return CreatePrimaryCommandBuffers(type, 1)[0];
}

vk::CommandBuffer CommandBufferManager::CreateSecondaryCommandBuffer(CommandBufferType type)
{
    return CreateSecondaryCommandBuffers(type, 1)[0];
}

std::vector<vk::CommandBuffer> CommandBufferManager::CreatePrimaryCommandBuffers(CommandBufferType type, uint32_t count)
{
    vk::CommandBufferAllocateInfo commandBufferAllocateInfo;
    commandBufferAllocateInfo.setLevel(vk::CommandBufferLevel::ePrimary)
        .setCommandPool(mCommandPools[type].get())
        .setCommandBufferCount(count);
    auto buffers = mContext->GetDevice().allocateCommandBuffersUnique(commandBufferAllocateInfo);
    if (buffers.size() != count)
    {
        mLogger->Error("Failed to allocate command buffers.");
        throw std::runtime_error("Failed to allocate command buffers.");
    }
    mPrimaryBuffers[type].reserve(count);

    std::vector<vk::CommandBuffer> result;
    for (auto &buffer : buffers)
    {
        mPrimaryBuffers[type].push_back(std::move(buffer));
        result.push_back(mPrimaryBuffers[type].back().get());
    }
    mLogger->Debug("Allocated {} primary command buffers.", std::to_string(count));
    return result;
}

std::vector<vk::CommandBuffer> CommandBufferManager::CreateSecondaryCommandBuffers(CommandBufferType type,
                                                                                   uint32_t count)
{
    vk::CommandBufferAllocateInfo commandBufferAllocateInfo;
    commandBufferAllocateInfo.setLevel(vk::CommandBufferLevel::eSecondary)
        .setCommandPool(mCommandPools[type].get())
        .setCommandBufferCount(count);

    auto buffers = mContext->GetDevice().allocateCommandBuffersUnique(commandBufferAllocateInfo);
    if (buffers.size() != count)
    {
        mLogger->Error("Failed to allocate command buffers.");
        throw std::runtime_error("Failed to allocate command buffers.");
    }
    mSecondaryBuffers[type].reserve(count);
    std::vector<vk::CommandBuffer> result;
    for (auto &buffer : buffers)
    {
        mSecondaryBuffers[type].push_back(std::move(buffer));
        result.push_back(mSecondaryBuffers[type].back().get());
    }
    mLogger->Debug("Allocated {} primary command buffers.", std::to_string(count));
    return result;
}
} // namespace MEngine