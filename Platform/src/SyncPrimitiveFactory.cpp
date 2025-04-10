#include "SyncPrimitiveManager.hpp"

namespace MEngine
{
SyncPrimitiveManager::SyncPrimitiveManager(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context)
    : mLogger(logger), mContext(context)
{
    // Create fences and semaphores if needed
}
vk::UniqueFence SyncPrimitiveManager::CreateFence(vk::FenceCreateFlags flags)
{
    vk::FenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.setFlags(flags);
    auto fence = mContext->GetDevice().createFenceUnique(fenceCreateInfo);
    if (!fence)
    {
        mLogger->Error("Failed to create fence");
        throw std::runtime_error("Failed to create fence");
    }
    mLogger->Debug("Fence created with flags: ", vk::to_string(flags));
    return fence;
}

vk::UniqueSemaphore SyncPrimitiveManager::CreateUniqueSemaphore()
{
    vk::SemaphoreCreateInfo semaphoreCreateInfo{};
    auto semaphore = mContext->GetDevice().createSemaphoreUnique(semaphoreCreateInfo);
    if (!semaphore)
    {
        mLogger->Error("Failed to create semaphore");
        throw std::runtime_error("Failed to create semaphore");
    }
    mLogger->Debug("Semaphore created");
    return semaphore;
}
} // namespace MEngine