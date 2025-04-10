#include "SyncPrimitiveManager.hpp"

namespace MEngine
{
SyncPrimitiveManager::SyncPrimitiveManager(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context)
    : mLogger(logger), mContext(context)
{
    // Create fences and semaphores if needed
}
vk::Fence SyncPrimitiveManager::CreateFence(vk::FenceCreateFlags flags)
{
    vk::FenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.setFlags(flags);
    auto fence = mContext->GetDevice().createFenceUnique(fenceCreateInfo);
    if (!fence)
    {
        mLogger->Error("Failed to create fence");
        throw std::runtime_error("Failed to create fence");
    }
    mFences.push_back(std::move(fence));
    mLogger->Debug("Fence created with flags: ", vk::to_string(flags));
    return mFences.back().get();
}

vk::Semaphore SyncPrimitiveManager::CreateUniqueSemaphore()
{
    vk::SemaphoreCreateInfo semaphoreCreateInfo{};
    auto semaphore = mContext->GetDevice().createSemaphoreUnique(semaphoreCreateInfo);
    if (!semaphore)
    {
        mLogger->Error("Failed to create semaphore");
        throw std::runtime_error("Failed to create semaphore");
    }
    mSemaphores.push_back(std::move(semaphore));
    mLogger->Debug("Semaphore created");
    return mSemaphores.back().get();
}
} // namespace MEngine