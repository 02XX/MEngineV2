#include "Material.hpp"

namespace MEngine
{
Material::Material(PipelineType type, vk::Pipeline pipeline, vk::DescriptorSet descriptorSet)
    : mPipeline(pipeline), mDescriptorSet(descriptorSet), mPipelineType(type)
{
}
vk::Pipeline Material::GetPipeline() const
{
    return mPipeline;
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