#pragma once
#include "NoCopyable.hpp"
#include "PipelineLayoutManager.hpp"
#include "PipelineManager.hpp"
#include <cstdint>
#include <unordered_map>
#include <vector>
namespace MEngine
{
class Material final : public NoCopyable
{
  public:
    Material() = default;

  private:
    PipelineType pipelineType;             // 渲染管线类型
    PipelineLayoutType pipelineLayoutType; // 管线布局类型
    vk::UniqueDescriptorSet descriptorSet; // 描述符集
};
} // namespace MEngine