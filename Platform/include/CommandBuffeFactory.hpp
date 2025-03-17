#pragma once
#include "Context.hpp"
#include "MEngine.hpp"
#include <vector>
#include <vulkan/vulkan.hpp>
namespace MEngine
{
class MENGINE_API CommandBufferFactory final
{
  private:
    uint32_t mQueueFamilyIndex;
    vk::UniqueCommandPool mCommandPool;

  public:
    CommandBufferFactory(uint32_t queueFamilyIndex);

    vk::UniqueCommandBuffer CreatePrimaryCommandBuffer();
    vk::UniqueCommandBuffer CreateSecondaryCommandBuffer();
    std::vector<vk::UniqueCommandBuffer> CreatePrimaryCommandBuffers(uint32_t count);
    std::vector<vk::UniqueCommandBuffer> CreateSecondaryCommandBuffers(uint32_t count);
};
} // namespace MEngine