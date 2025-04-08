#pragma once
#include "MEngine.hpp"
#include "PipelineLayoutManager.hpp"
#include "PipelineManager.hpp"
namespace MEngine
{
struct MaterialComponent
{
    PipelineType pipelineType;             // 渲染管线类型
    PipelineLayoutType pipelineLayoutType; // 管线布局类型
    
};
} // namespace MEngine