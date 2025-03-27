#include "Mesh.hpp"

namespace MEngine
{
Mesh::Mesh(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices)
    : mVertices(vertices), mIndices(indices)
{
    mVertexBuffer = mBufferManager.CreateUniqueVertexBuffer(sizeof(Vertex) * mVertices.size(), mVertices.data());
    mIndexBuffer = mBufferManager.CreateUniqueIndexBuffer(sizeof(uint32_t) * mIndices.size(), mIndices.data());
}
vk::Buffer Mesh::GetVertexBuffer() const
{
    return mVertexBuffer->GetBuffer();
}
vk::Buffer Mesh::GetIndexBuffer() const
{
    return mIndexBuffer->GetBuffer();
}
uint32_t Mesh::GetIndexCount() const
{
    return static_cast<uint32_t>(mIndices.size());
}
} // namespace MEngine