#include "Mesh.hpp"
#include "BufferFactory.hpp"

namespace MEngine
{
Mesh::Mesh(std::shared_ptr<BufferFactory> bufferFactory, const std::vector<Vertex> &vertices,
           const std::vector<uint32_t> &indices)
    : mVertices(vertices), mIndices(indices), mBufferFactory(bufferFactory)
{
    // 创建顶点缓冲区
    mVertexBuffer =
        mBufferFactory->CreateBuffer(BufferType::Vertex, sizeof(Vertex) * mVertices.size(), mVertices.data());
    // 创建索引缓冲区
    mIndexBuffer = mBufferFactory->CreateBuffer(BufferType::Index, sizeof(uint32_t) * mIndices.size(), mIndices.data());
}
vk::Buffer Mesh::GetVertexBuffer() const
{
    return mVertexBuffer->GetHandle();
}
vk::Buffer Mesh::GetIndexBuffer() const
{
    return mIndexBuffer->GetHandle();
}
uint32_t Mesh::GetIndexCount() const
{
    return static_cast<uint32_t>(mIndices.size());
}
} // namespace MEngine