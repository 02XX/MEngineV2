#pragma once
#include "Buffer.hpp"
#include "CommandBuffeManager.hpp"
#include "Context.hpp"
#include "Logger.hpp"
#include "MEngine.hpp"
#include "SharedHandle.hpp"
#include <cstdint>
#include <memory>
namespace MEngine
{
class MENGINE_API MeshComponent final
{
  private:
    UniqueBuffer mVertexBuffer;
    UniqueBuffer mIndexBuffer;
    UniqueCommandBuffer mCommandBuffer;
    std::shared_ptr<CommandBufferManager> mCommandBufferManager;
    uint32_t mMeshID;
    uint32_t mIndexCount = 0;                          // 索引数量
    uint32_t mVertexOffset = 0;                        // 顶点偏移
    uint32_t mFirstIndex = 0;                          // 首索引偏移
    vk::IndexType mIndexType = vk::IndexType::eUint32; // 索引类型
  public:
    MeshComponent(std::shared_ptr<CommandBufferManager> commandBufferManager);
    void SetVertexBuffer(UniqueBuffer buffer);
    void SetIndexBuffer(UniqueBuffer buffer);
    vk::CommandBuffer Record() const noexcept;

    uint32_t GetID() const noexcept;
    uint32_t GetIndexCount() const noexcept;
    uint32_t GetVertexOffset() const noexcept;
    uint32_t GetFirstIndex() const noexcept;
    vk::IndexType GetIndexType() const noexcept;
    const UniqueBuffer &GetVertexBuffer() const noexcept;
    const UniqueBuffer &GetIndexBuffer() const noexcept;
};
} // namespace MEngine