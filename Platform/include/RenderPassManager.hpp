#pragma once

#include "Context.hpp"
#include "Logger.hpp"
#include "MEngine.hpp"
#include "NoCopyable.hpp"
#include <vulkan/vulkan.hpp>

namespace MEngine
{
struct SubpassConfig
{
    // 当前子流程的颜色附件引用
    std::vector<vk::AttachmentReference> colorAttachments;

    // 输入附件引用（从前面子流程读取）
    std::vector<vk::AttachmentReference> inputAttachments;

    // 解析附件引用（将MSAA结果解析到单采样附件）
    std::vector<vk::AttachmentReference> resolveAttachments;

    // 需保留的附件索引（不被当前子流程修改）
    std::vector<uint32_t> preserveAttachments;

    // 深度模板附件引用（可选）
    std::optional<vk::AttachmentReference> depthStencilAttachment;

    // 其他 Vulkan 子流程参数
    vk::PipelineBindPoint bindPoint = vk::PipelineBindPoint::eGraphics;
};
struct RenderPassConfigInfo
{
    // 所有附件描述（全局）
    std::vector<vk::AttachmentDescription> attachments;

    // 子流程配置数组（每个元素对应一个子流程）
    std::vector<SubpassConfig> subpasses;

    // 子流程间依赖关系
    std::vector<vk::SubpassDependency> dependencies;
};
class RenderPassManager final : public NoCopyable
{
  public:
    RenderPassManager() = default;
    vk::UniqueRenderPass CreateRenderPass(const RenderPassConfigInfo &config);
    vk::UniqueRenderPass CreateSimpleRenderPass(vk::Format colorFormat,           // 颜色附件格式
                                                vk::ImageLayout colorFinalLayout, // 颜色最终布局（如ePresentSrcKHR）
                                                vk::Format depthFormat            // 深度附件格式
    );
    vk::UniqueRenderPass CreateMSAARenderPass(vk::Format colorFormat, vk::ImageLayout colorFinalLayout,
                                              vk::Format resolveFormat, vk::Format depthFormat,
                                              vk::SampleCountFlagBits samples);
};

} // namespace MEngine