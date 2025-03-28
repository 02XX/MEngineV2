#pragma once
#include "Buffer.hpp"
#include "BufferManager.hpp"
#include "MEngine.hpp"
#include "PipelineManager.hpp"
#include "Vertex.hpp"

namespace MEngine
{
class Material
{
  private:
    PipelineType mPipelineType;
    vk::DescriptorSet mDescriptorSet;

  public:
    Material(PipelineType type, vk::DescriptorSet descriptorSet);
    vk::DescriptorSet GetDescriptorSet() const;
    PipelineType GetPipelineType() const;
};
} // namespace MEngine