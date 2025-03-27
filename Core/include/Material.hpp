#pragma once
#include "Buffer.hpp"
#include "BufferManager.hpp"
#include "MEngine.hpp"
#include "PipelineManager.hpp"
#include "Vertex.hpp"

namespace MEngine
{
class MENGINE_API Material
{
  private:
    PipelineType mPipelineType;
    vk::Pipeline mPipeline;
    vk::DescriptorSet mDescriptorSet;

  public:
    Material(PipelineType type, vk::Pipeline pipeline, vk::DescriptorSet descriptorSet);
    vk::Pipeline GetPipeline() const;
    vk::DescriptorSet GetDescriptorSet() const;
    PipelineType GetPipelineType() const;
};
} // namespace MEngine