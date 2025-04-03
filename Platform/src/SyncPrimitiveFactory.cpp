#include "SyncPrimitiveManager.hpp"

namespace MEngine
{
vk::UniqueFence SyncPrimitiveManager::CreateFence(vk::FenceCreateFlags flags) const
{
    auto &context = Context::Instance();
    vk::FenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.setFlags(flags);
    auto fence = context.GetDevice().createFenceUnique(fenceCreateInfo);
    if (!fence)
    {
        LogE("Failed to create fence");
        throw std::runtime_error("Failed to create fence");
    }
    LogD("Fence created with flags: ", vk::to_string(flags));
    return fence;
}

vk::UniqueSemaphore SyncPrimitiveManager::CreateUniqueSemaphore() const
{
    auto &context = Context::Instance();
    vk::SemaphoreCreateInfo semaphoreCreateInfo{};
    auto semaphore = context.GetDevice().createSemaphoreUnique(semaphoreCreateInfo);
    if (!semaphore)
    {
        LogE("Failed to create semaphore");
        throw std::runtime_error("Failed to create semaphore");
    }
    LogD("Semaphore created");
    return semaphore;
}
} // namespace MEngine