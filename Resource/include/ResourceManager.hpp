#pragma once
#include "Logger.hpp"
#include "MEngine.hpp"
#include "PipelineManager.hpp"
#include "SharedHandle.hpp"
#include <vector>
namespace MEngine
{
enum class ResourceType
{
    Texture,
    Pipeline,
    Mesh,
    Material,
    Shader
};
enum class PipelineType
{
    ForwardOpaque,   // 前向渲染（不透明物体）
    DeferredGBuffer, // 延迟渲染-GBuffer阶段
    ShadowDepth,     // 阴影深度渲染
    PostProcess      // 后处理（可按需扩展）
};
class MENGINE_API ResourceManager
{
  private:
    PipelineManager mPipelineManager;
    std::map<PipelineType, UniquePipeline> mPipelines;

  public:
    ResourceManager();
    ~ResourceManager();
    void LoadPipeline();
    vk::Pipeline &GetPipeline(PipelineType type) const;
};
} // namespace MEngine