#pragma once
#include "Context.hpp"
#include "Logger.hpp"
#include "MEngine.hpp"
#include "NoCopyable.hpp"
#include "PipelineLayoutManager.hpp"
#include "RenderPassManager.hpp"
#include "ShaderManager.hpp"
#include "SharedHandle.hpp"
#include "Vertex.hpp"
#include <memory>
#include <vulkan/vulkan.hpp>

namespace MEngine
{
enum class PipelineType
{
    DefferGBuffer,  // 延迟渲染的几何缓冲区生成
    ShadowDepth,    // 阴影深度图渲染
    DefferLighting, // 光照计算（直接/间接光）
    Translucency,   // 半透明物体渲染
    PostProcess,    // 后处理特效
    Sky,            // 天空盒/大气渲染
    UI              // 界面渲染
};
class PipelineManager final : public NoCopyable
{
  private:
    std::unordered_map<PipelineType, vk::UniquePipeline> mPipelines;
    std::shared_ptr<ShaderManager> mShaderManager;
    std::shared_ptr<PipelineLayoutManager> mPipelineLayoutManager;
    std::shared_ptr<RenderPassManager> mRenderPassManager;

  private:
    void CommonSetting();
    void CreateGBufferPipeline();
    void CreateShadowDepthPipeline();
    void CreateLightingPipeline();
    void CreateTranslucencyPipeline();
    void CreatePostProcessPipeline();
    void CreateSkyPipeline();
    void CreateUIPipeline();

  private:
    std::vector<vk::VertexInputAttributeDescription> mVertexAttributeDescriptions;
    vk::VertexInputBindingDescription mVertexBindingDescription;
    vk::PipelineVertexInputStateCreateInfo mVertexInputInfo;
    vk::PipelineInputAssemblyStateCreateInfo mInputAssemblyInfo;
    vk::PipelineViewportStateCreateInfo mViewportInfo;

    vk::PipelineRasterizationStateCreateInfo mRasterizationInfo;
    vk::Viewport mViewport;
    vk::Rect2D mScissor;

    vk::PipelineMultisampleStateCreateInfo mMultisampleInfo;
    vk::PipelineDepthStencilStateCreateInfo mDepthStencilInfo;
    vk::PipelineColorBlendStateCreateInfo mColorBlendInfo;

    vk::GraphicsPipelineCreateInfo mConfig;

  public:
    PipelineManager(std::shared_ptr<ShaderManager> shaderManager,
                    std::shared_ptr<PipelineLayoutManager> pipelineLayoutManager,
                    std::shared_ptr<RenderPassManager> renderPassManager);
    vk::Pipeline GetPipeline(PipelineType type) const;
};
} // namespace MEngine
