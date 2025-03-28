#include "SyncPrimitiveManager.hpp"

namespace MEngine
{
vk::UniqueFence SyncPrimitiveManager::CreateFence(vk::FenceCreateFlags flags) const
{
    auto &context = Context::Instance();
    vk::FenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.setFlags(flags);
    auto fence = context.GetDevice().createFenceUnique(fenceCreateInfo);
    LogD("Fence created with flags: ", vk::to_string(flags));
    return fence;
}

vk::UniqueSemaphore SyncPrimitiveManager::CreateUniqueSemaphore() const
{
    auto &context = Context::Instance();
    vk::SemaphoreCreateInfo semaphoreCreateInfo{};
    auto semaphore = context.GetDevice().createSemaphoreUnique(semaphoreCreateInfo);
    LogD("Semaphore created");
    return semaphore;
}
} // namespace MEngine