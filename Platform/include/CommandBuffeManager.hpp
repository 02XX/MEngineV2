#pragma once
#include "Context.hpp"
#include "MEngine.hpp"
#include "NoCopyable.hpp"
#include <vector>
#include <vulkan/vulkan.hpp>

namespace MEngine
{
using UniqueCommandBuffer = vk::UniqueCommandBuffer;
class CommandBufferManager final : public NoCopyable
{
  private:
    uint32_t mQueueFamilyIndex;
    vk::UniqueCommandPool mCommandPool;

  public:
    CommandBufferManager(uint32_t queueFamilyIndex);
    /**
     * @brief Create a Primary Command Buffer object
     *
     * @return vk::UniqueCommandBuffer
     */
    vk::UniqueCommandBuffer CreatePrimaryCommandBuffer();
    /**
     * @brief Create a Secondary Command Buffer object
     *
     * @return vk::UniqueCommandBuffer
     */
    vk::UniqueCommandBuffer CreateSecondaryCommandBuffer();
    /**
     * @brief Create multiple Primary Command Buffers object
     *
     * @param count Create by the count number
     * @return std::vector<vk::UniqueCommandBuffer>
     */
    std::vector<vk::UniqueCommandBuffer> CreatePrimaryCommandBuffers(uint32_t count);
    /**
     * @brief Create multiple Secondary Command Buffers object
     *
     * @param count Create by the count number
     * @return std::vector<vk::UniqueCommandBuffer>
     */
    std::vector<vk::UniqueCommandBuffer> CreateSecondaryCommandBuffers(uint32_t count);
};
} // namespace MEngine