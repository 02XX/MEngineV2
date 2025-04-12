#pragma once
#include "MEngine.hpp"
#include "Material.hpp"
#include "PipelineLayoutManager.hpp"
#include "PipelineManager.hpp"
#include <memory>
namespace MEngine
{
struct MaterialComponent
{
    // PipelineType pipelineType;             // 渲染管线类型
    // PipelineLayoutType pipelineLayoutType; // 管线布局类型
    std::shared_ptr<Material> material; // 材质
    
};
} // namespace MEngine