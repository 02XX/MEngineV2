#pragma once
#include "Context.hpp"
#include "Interface/ILogger.hpp"
#include "MEngine.hpp"
#include "NoCopyable.hpp"
#include <memory>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace MEngine
{
// 并不是和PipelineType一一对应的关系，PipelineLayoutType是为了方便管理而定义的
// 例如当piplineType为PostProcessBloom和PostProcessTAA，但是PipelineLayoutType可以为PostProcess，
// 因为他们两个管线的Shader（ShaderModule一样）和资源（描述符资源一样）是一样的
enum class PipelineLayoutType
{
    DefferLayout,       // 延迟渲染
    ShadowDepthLayout,  // 阴影深度图渲染
    TranslucencyLayout, // 半透明物体渲染
    PostProcessLayout,  // 后处理特效
    SkyLayout,          // 天空盒/大气渲染
    UILayout            // 界面渲染
};

class Layout
{
    vk::DescriptorSet MVPDescriptorSet;
};
class DefferLayout : public Layout
{
    vk::DescriptorSet AlbedoDescriptorSet;
    vk::DescriptorSet NormalDescriptorSet;
    vk::DescriptorSet MetalRoughDescriptorSet;
    vk::DescriptorSet AODescriptorSet;
};
class TranslucencyLayout : public Layout
{
};

class PipelineLayoutManager final : public NoCopyable
{
  private:
    // DI
    std::shared_ptr<ILogger> mLogger;
    std::shared_ptr<Context> mContext;

  private:
    std::unordered_map<PipelineLayoutType, vk::UniquePipelineLayout> mPipelineLayouts;
    std::unordered_map<PipelineLayoutType, vk::UniqueDescriptorSetLayout> mDescriptorSetLayouts;

    std::vector<std::weak_ptr<Layout>> mLayouts;
    void CreateDefferPipelineLayout();
    void CreateShadowDepthPipelineLayout();
    void CreateTranslucencyPipelineLayout();
    void CreatePostProcessPipelineLayout();
    void CreateSkyPipelineLayout();
    void CreateUIPipelineLayout();

    // void CreateDefferDescriptorSet();
    // void CreateShadowDepthDescriptorSet();
    // void CreateTranslucencyDescriptorSet();
    // void CreatePostProcessDescriptorSet();
    // void CreateSkyDescriptorSet();
    // void CreateUIDescriptorSet();

  public:
    PipelineLayoutManager(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context);
    vk::PipelineLayout GetPipelineLayout(PipelineLayoutType type) const;
    vk::DescriptorSetLayout GetDescriptorSetLayout(PipelineLayoutType type) const;
    std::shared_ptr<Layout> GetLayout(PipelineLayoutType type) const;
};

} // namespace MEngine