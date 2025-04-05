#pragma once
#include "Context.hpp"
#include "MEngine.hpp"
#include "NoCopyable.hpp"
#include <vulkan/vulkan.hpp>

namespace MEngine
{
class SyncPrimitiveManager final : public NoCopyable
{
  private:
    // DI
    std::shared_ptr<ILogger> mLogger;
    std::shared_ptr<Context> mContext;

  public:
    SyncPrimitiveManager(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context);
    vk::UniqueFence CreateFence(vk::FenceCreateFlags flags = {}) const;
    vk::UniqueSemaphore CreateUniqueSemaphore() const;
};
} // namespace MEngine
