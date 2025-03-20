#include "Componet/MeshComponent.hpp"

namespace MEngine
{
MeshComponent::MeshComponent(std::shared_ptr<CommandBufferManager> commandBufferManager)
    : mCommandBufferManager(commandBufferManager)
{
    mCommandBuffer = mCommandBufferManager->CreateSecondaryCommandBuffer();
}
void MeshComponent::SetVertexBuffer(UniqueBuffer buffer)
{
    mVertexBuffer = std::move(buffer);
}
void MeshComponent::SetIndexBuffer(UniqueBuffer buffer)
{
    mIndexBuffer = std::move(buffer);
}
vk::CommandBuffer MeshComponent::Record() const noexcept
{
}

uint32_t MeshComponent::GetID() const noexcept
{
    return mMeshID;
}
uint32_t MeshComponent::GetIndexCount() const noexcept
{
    return mIndexCount;
}
uint32_t MeshComponent::GetVertexOffset() const noexcept
{
    return mVertexOffset;
}
uint32_t MeshComponent::GetFirstIndex() const noexcept
{
    return mFirstIndex;
}
vk::IndexType MeshComponent::GetIndexType() const noexcept
{
    return mIndexType;
}
const UniqueBuffer &MeshComponent::GetVertexBuffer() const noexcept
{
    return mVertexBuffer;
}
const UniqueBuffer &MeshComponent::GetIndexBuffer() const noexcept
{
    return mIndexBuffer;
}
} // namespace MEngine