#include "FrameBufferManager.hpp"

namespace MEngine
{
FrameBufferManager::FrameBufferManager(vk::RenderPass renderPass, vk::Extent2D extent,
                                       const std::vector<vk::ImageView> &attachments)
{
    auto &context = Context::Instance();
    vk::FramebufferCreateInfo framebufferCreateInfo;
    framebufferCreateInfo.setRenderPass(renderPass)
        .setAttachments(attachments)
        .setWidth(extent.width)
        .setHeight(extent.height)
        .setLayers(1);
    mFramebuffer = context.GetDevice()->createFramebufferUnique(framebufferCreateInfo);
    LogI("Framebuffer Created.");
}

vk::Framebuffer FrameBufferManager::GetFramebuffer() const
{
    return mFramebuffer.get();
}
} // namespace MEngine