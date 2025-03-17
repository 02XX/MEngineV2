#pragma once
#include "Context.hpp"
#include "MEngine.hpp"
#include <vulkan/vulkan.hpp>

namespace MEngine
{
class MENGINE_API SyncPrimitiveManager final
{
  public:
    SyncPrimitiveManager() = default;
    vk::UniqueFence CreateFence(vk::FenceCreateFlags flags = {}) const;
    vk::UniqueSemaphore CreateUniqueSemaphore() const;
};
} // namespace MEngine
