#pragma once
#include "Context.hpp"
#include "Interface/ILogger.hpp"
#include "MEngine.hpp"
#include "NoCopyable.hpp"
#include <memory>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace MEngine
{
enum class CommandBufferType
{
    Graphic,
    Transfer,
    Present
};
class CommandBufferManager final : public NoCopyable
{
  private:
    // DI
    std::shared_ptr<Context> mContext;
    std::shared_ptr<ILogger> mLogger;

  private:
    std::unordered_map<CommandBufferType, vk::UniqueCommandPool> mCommandPools;

  public:
    CommandBufferManager(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context);
    /**
     * @brief Create a Primary Command Buffer object
     *
     * @return vk::UniqueCommandBuffer
     */
    vk::UniqueCommandBuffer CreatePrimaryCommandBuffer(CommandBufferType type);
    /**
     * @brief Create a Secondary Command Buffer object
     *
     * @return vk::UniqueCommandBuffer
     */
    vk::UniqueCommandBuffer CreateSecondaryCommandBuffer(CommandBufferType type);
    /**
     * @brief Create multiple Primary Command Buffers object
     *
     * @param count Create by the count number
     * @return std::vector<vk::UniqueCommandBuffer>
     */
    std::vector<vk::UniqueCommandBuffer> CreatePrimaryCommandBuffers(CommandBufferType type, uint32_t count);
    /**
     * @brief Create multiple Secondary Command Buffers object
     *
     * @param count Create by the count number
     * @return std::vector<vk::UniqueCommandBuffer>
     */
    std::vector<vk::UniqueCommandBuffer> CreateSecondaryCommandBuffers(CommandBufferType type, uint32_t count);
};
} // namespace MEngine