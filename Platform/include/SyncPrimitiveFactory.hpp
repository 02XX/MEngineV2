#pragma once
#include "Context.hpp"
#include "MEngine.hpp"
#include <vulkan/vulkan.hpp>

namespace MEngine
{
class MENGINE_API SyncPrimitiveFactory final
{
  public:
    SyncPrimitiveFactory() = default;
    vk::UniqueFence CreateFence(vk::FenceCreateFlags flags = {}) const;
    vk::UniqueSemaphore CreateUniqueSemaphore() const;
};
} // namespace MEngine
