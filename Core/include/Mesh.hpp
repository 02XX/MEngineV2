#pragma once
#include "Buffer.hpp"
#include "BufferManager.hpp"
#include "MEngine.hpp"
#include "Vertex.hpp"

namespace MEngine
{
class Mesh
{
  private:
    std::vector<Vertex> mVertices;
    std::vector<uint32_t> mIndices;
    UniqueBuffer mVertexBuffer; // Vulkan 顶点缓冲区（由资源管理器填充）
    UniqueBuffer mIndexBuffer;  // Vulkan 索引缓冲区
    std::shared_ptr<BufferManager> mBufferManager;

  public:
    Mesh(std::shared_ptr<BufferManager> bufferManager, const std::vector<Vertex> &vertices,
         const std::vector<uint32_t> &indices);
    vk::Buffer GetVertexBuffer() const;
    vk::Buffer GetIndexBuffer() const;
    uint32_t GetIndexCount() const;
};
} // namespace MEngine