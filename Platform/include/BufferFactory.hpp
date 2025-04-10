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
    Vertex,  // 顶点缓冲区
    Index,   // 索引缓冲区
    Uniform, // Uniform 缓冲区
    Staging, // 临时缓冲区
    Storage  // 存储缓冲区
};
class BufferFactory final : public NoCopyable
{
  private:
    // DI
    std::shared_ptr<Context> mContext;
    std::shared_ptr<ILogger> mLogger;

    std::shared_ptr<CommandBufferManager> mCommandBufferManager;
    std::shared_ptr<SyncPrimitiveManager> mSyncPrimitiveManager;

  private:
    vk::CommandBuffer mCommandBuffer;
    vk::Fence mFence;

  public:
    BufferFactory(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                  std::shared_ptr<CommandBufferManager> commandBufferManager,
                  std::shared_ptr<SyncPrimitiveManager> syncPrimitiveManager);
    UniqueBuffer CreateBuffer(BufferType type, vk::DeviceSize size, const void *data = nullptr);
    void CopyBuffer(Buffer *src, Buffer *dst);
};
} // namespace MEngine
