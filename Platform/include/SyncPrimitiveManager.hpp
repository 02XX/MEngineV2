#pragma once
#include "Context.hpp"
#include "MEngine.hpp"
#include "NoCopyable.hpp"
#include <vulkan/vulkan.hpp>

namespace MEngine
{
class SyncPrimitiveManager final : public NoCopyable
{
  public:
    SyncPrimitiveManager() = default;
    vk::UniqueFence CreateFence(vk::FenceCreateFlags flags = {}) const;
    vk::UniqueSemaphore CreateUniqueSemaphore() const;
};
} // namespace MEngine
