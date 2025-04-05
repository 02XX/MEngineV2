#pragma once
#include "Context.hpp"
#include "Interface/ILogger.hpp"
#include "MEngine.hpp"
#include "NoCopyable.hpp"
#include <memory>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace MEngine
{

class CommandBufferManager final : public NoCopyable
{
  private:
    // DI
    std::shared_ptr<Context> mContext;
    std::shared_ptr<ILogger> mLogger;

  private:
    uint32_t mQueueFamilyIndex;
    vk::UniqueCommandPool mCommandPool;

  public:
    CommandBufferManager(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context, uint32_t queueFamilyIndex);
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