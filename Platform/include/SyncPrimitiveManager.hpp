#pragma once
#include "Context.hpp"
#include "MEngine.hpp"
#include "NoCopyable.hpp"
#include <vector>
#include <vulkan/vulkan.hpp>

namespace MEngine
{
class SyncPrimitiveManager final : public NoCopyable
{
  private:
    // DI
    std::shared_ptr<ILogger> mLogger;
    std::shared_ptr<Context> mContext;

  private:
    std::vector<vk::UniqueFence> mFences;
    std::vector<vk::UniqueSemaphore> mSemaphores;

  public:
    SyncPrimitiveManager(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context);
    vk::Fence CreateFence(vk::FenceCreateFlags flags = {});
    vk::Semaphore CreateUniqueSemaphore();
};
} // namespace MEngine
