#include "Material.hpp"

namespace MEngine
{
Material::Material(PipelineType type, vk::DescriptorSet descriptorSet)
    : mDescriptorSet(descriptorSet), mPipelineType(type)
{
}
vk::DescriptorSet Material::GetDescriptorSet() const
{
    return mDescriptorSet;
}
PipelineType Material::GetPipelineType() const
{
    return mPipelineType;
}
} // namespace MEngine