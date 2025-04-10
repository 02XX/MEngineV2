#pragma once
#include "Buffer.hpp"
#include "CommandBuffeManager.hpp"
#include "MEngine.hpp"
#include "NoCopyable.hpp"
#include "SyncPrimitiveManager.hpp"
#include "VMA.hpp"
#include <memory>
#include <vulkan/vulkan.hpp>

namespace MEngine
{
enum class BufferType
{
    Vertex,
    Index,
    Uniform,
    Staging,
    Storage
};
class BufferFactory final : public NoCopyable
{
  private:
    // DI
    std::shared_ptr<Context> mContext;
    std::shared_ptr<ILogger> mLogger;

    std::shared_ptr<CommandBufferManager> mCommandBufferManager;
    std::shared_ptr<SyncPrimitiveManager> mSyncPrimitiveManager;

  public:
    BufferFactory(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                  std::shared_ptr<CommandBufferManager> commandBufferManager,
                  std::shared_ptr<SyncPrimitiveManager> syncPrimitiveManager);
    UniqueBuffer CreateBuffer(BufferType type, vk::DeviceSize size, const void *data = nullptr);
    void CopyBuffer(Buffer *src, Buffer *dst, vk::DeviceSize size);
};
} // namespace MEngine
