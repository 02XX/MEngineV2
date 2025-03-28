#include "RenderPassManager.hpp"
#include <vulkan/vulkan.hpp>

namespace MEngine
{
vk::UniqueRenderPass RenderPassManager::CreateRenderPass(const RenderPassConfigInfo &config)
{
    auto &context = Context::Instance();

    std::vector<vk::SubpassDescription> subpasses;
    for (auto &subpassConfig : config.subpasses)
    {
        vk::SubpassDescription subpassDescription;
        subpassDescription.setPipelineBindPoint(subpassConfig.bindPoint)
            .setColorAttachments(subpassConfig.colorAttachments)
            .setInputAttachments(subpassConfig.inputAttachments)
            .setPreserveAttachments(subpassConfig.preserveAttachments);
        if (subpassConfig.depthStencilAttachment.has_value())
        {
            subpassDescription.setPDepthStencilAttachment(&subpassConfig.depthStencilAttachment.value());
        }
        subpasses.push_back(subpassDescription);
    }

    vk::RenderPassCreateInfo renderPassCreateInfo{};
    renderPassCreateInfo.setSubpasses(subpasses)
        .setAttachments(config.attachments)
        .setDependencies(config.dependencies);

    auto renderPass = context.GetDevice().createRenderPassUnique(renderPassCreateInfo);
    LogI("Render Pass Created.");
    return renderPass;
}

vk::UniqueRenderPass RenderPassManager::CreateSimpleRenderPass(vk::Format colorFormat, vk::ImageLayout colorFinalLayout,
                                                               vk::Format depthFormat)
{
    RenderPassConfigInfo config{};
    config.attachments = {// 颜色附件 (索引0)
                          vk::AttachmentDescription()
                              .setFormat(colorFormat)                    // 颜色格式
                              .setSamples(vk::SampleCountFlagBits::e1)   // 无多重采样
                              .setLoadOp(vk::AttachmentLoadOp::eClear)   // 初始清空颜色
                              .setStoreOp(vk::AttachmentStoreOp::eStore) // 渲染后存储结果
                              .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                              .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                              .setInitialLayout(vk::ImageLayout::eUndefined)
                              .setFinalLayout(colorFinalLayout), // 输出

                          // 深度附件 (索引1)
                          vk::AttachmentDescription()
                              .setFormat(depthFormat) // 32位深度 + 8位模板
                              .setSamples(vk::SampleCountFlagBits::e1)
                              .setLoadOp(vk::AttachmentLoadOp::eClear)      // 初始清空深度
                              .setStoreOp(vk::AttachmentStoreOp::eDontCare) // 渲染后不保留深度数据
                              .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                              .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                              .setInitialLayout(vk::ImageLayout::eUndefined)
                              .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)};
    config.subpasses.resize(1);
    auto &subpass = config.subpasses[0];
    // 颜色附件引用（对应索引0）
    subpass.colorAttachments = {vk::AttachmentReference()
                                    .setAttachment(0) // 对应attachments数组索引0
                                    .setLayout(vk::ImageLayout::eColorAttachmentOptimal)};

    // 深度附件引用（对应索引1）
    subpass.depthStencilAttachment =
        vk::AttachmentReference().setAttachment(1).setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

    config.dependencies = {// 从外部到子流程0的依赖
                           vk::SubpassDependency()
                               .setSrcSubpass(vk::SubpassExternal)
                               .setDstSubpass(0)
                               .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                               .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput |
                                                vk::PipelineStageFlagBits::eLateFragmentTests)
                               .setSrcAccessMask(vk::AccessFlagBits::eNone)
                               .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite |
                                                 vk::AccessFlagBits::eDepthStencilAttachmentWrite)};
    return CreateRenderPass(config);
}

vk::UniqueRenderPass RenderPassManager::CreateMSAARenderPass(vk::Format colorFormat, vk::ImageLayout colorFinalLayout,
                                                             vk::Format resolveFormat, vk::Format depthFormat,
                                                             vk::SampleCountFlagBits samples)
{
    RenderPassConfigInfo config{};
    config.attachments = {// MSAA颜色附件（索引0）
                          vk::AttachmentDescription()
                              .setFormat(colorFormat)
                              .setSamples(samples)
                              .setLoadOp(vk::AttachmentLoadOp::eClear)
                              .setStoreOp(vk::AttachmentStoreOp::eDontCare)
                              .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                              .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                              .setInitialLayout(vk::ImageLayout::eUndefined)
                              .setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal),
                          // 深度附件（索引1）
                          vk::AttachmentDescription()
                              .setFormat(depthFormat)
                              .setSamples(samples)
                              .setLoadOp(vk::AttachmentLoadOp::eClear)
                              .setStoreOp(vk::AttachmentStoreOp::eDontCare)
                              .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                              .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                              .setInitialLayout(vk::ImageLayout::eUndefined)
                              .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal),
                          // 解析附件（索引2）
                          vk::AttachmentDescription()
                              .setFormat(resolveFormat)
                              .setSamples(vk::SampleCountFlagBits::e1)
                              .setLoadOp(vk::AttachmentLoadOp::eDontCare)
                              .setStoreOp(vk::AttachmentStoreOp::eStore)
                              .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                              .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                              .setInitialLayout(vk::ImageLayout::eUndefined)
                              .setFinalLayout(colorFinalLayout)};
    config.subpasses.resize(1);
    auto &subpass = config.subpasses[0];

    // 颜色附件引用（索引0）
    subpass.colorAttachments = {{0, vk::ImageLayout::eColorAttachmentOptimal}};

    // 深度附件引用（索引1）
    subpass.depthStencilAttachment = {1, vk::ImageLayout::eDepthStencilAttachmentOptimal};

    // 解析附件引用（索引2）
    subpass.resolveAttachments = {{2, vk::ImageLayout::eColorAttachmentOptimal}};

    config.dependencies = {vk::SubpassDependency()
                               .setSrcSubpass(vk::SubpassExternal)
                               .setDstSubpass(0)
                               .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput |
                                                vk::PipelineStageFlagBits::eEarlyFragmentTests)
                               .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput |
                                                vk::PipelineStageFlagBits::eEarlyFragmentTests)
                               .setSrcAccessMask(vk::AccessFlagBits::eNone)
                               .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite |
                                                 vk::AccessFlagBits::eDepthStencilAttachmentWrite)};
    return CreateRenderPass(config);
}

} // namespace MEngine