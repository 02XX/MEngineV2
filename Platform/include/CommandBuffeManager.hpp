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
    std::unordered_map<CommandBufferType, std::vector<vk::UniqueCommandBuffer>> mPrimaryBuffers;
    std::unordered_map<CommandBufferType, std::vector<vk::UniqueCommandBuffer>> mSecondaryBuffers;

  public:
    CommandBufferManager(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context);
    vk::UniqueCommandBuffer CreatePrimaryCommandBuffer(CommandBufferType type);
    vk::UniqueCommandBuffer CreateSecondaryCommandBuffer(CommandBufferType type);
    std::vector<vk::UniqueCommandBuffer> CreatePrimaryCommandBuffers(CommandBufferType type, uint32_t count);
    std::vector<vk::UniqueCommandBuffer> CreateSecondaryCommandBuffers(CommandBufferType type, uint32_t count);
};
} // namespace MEngine