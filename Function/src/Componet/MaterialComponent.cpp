#include "Componet/MaterialComponent.hpp"

namespace MEngine
{
void MaterialComponent::SetPipeline(SharedPipeline pipeline)
{
    mPipeline = std::move(pipeline);
}
void MaterialComponent::SetPipelineLayout(SharedPipelineLayout layout)
{
    mPipelineLayout = std::move(layout);
}
void MaterialComponent::SetDescriptorSet(SharedDescriptorSet set)
{
    mDescriptorSet = std::move(set);
}
uint32_t MaterialComponent::GetID() const noexcept
{
    return mMaterialID;
}
SharedPipeline MaterialComponent::GetPipeline() const noexcept
{
    return mPipeline;
}
SharedPipelineLayout MaterialComponent::GetPipelineLayout() const noexcept
{
    return mPipelineLayout;
}
SharedDescriptorSet MaterialComponent::GetDescriptorSet() const noexcept
{
    return mDescriptorSet;
}
} // namespace MEngine