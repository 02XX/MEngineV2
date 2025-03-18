#include "FrameBufferManager.hpp"

namespace MEngine
{
vk::UniqueFramebuffer FrameBufferManager::CreateFrameBuffer(vk::RenderPass renderPass, vk::Extent2D extent,
                                                            const std::vector<vk::ImageView> &attachments)
{
    auto &context = Context::Instance();
    vk::FramebufferCreateInfo framebufferCreateInfo;
    framebufferCreateInfo.setRenderPass(renderPass)
        .setAttachments(attachments)
        .setWidth(extent.width)
        .setHeight(extent.height)
        .setLayers(1);
    auto framebuffer = context.GetDevice()->createFramebufferUnique(framebufferCreateInfo);
    LogI("Framebuffer Created.");
    return framebuffer;
}
} // namespace MEngine