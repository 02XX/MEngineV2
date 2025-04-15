#include "RenderPassManager.hpp"
#include <array>
#include <vector>
#include <vulkan/vulkan_structs.hpp>

namespace MEngine
{
RenderPassManager::RenderPassManager(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                                     std::shared_ptr<ImageFactory> imageFactory)
    : mLogger(logger), mContext(context), mImageFactory(imageFactory)
{

    CreateShadowDepthRenderPass();
    CreateDeferredCompositionRenderPass();
    CreateForwardCompositionRenderPass();
    CreateSkyRenderPass();
    CreateTransparentRenderPass();
    CreatePostProcessRenderPass();
    CreateUIRenderPass();

    CreateShadowDepthFrameBuffer();
    CreateDeferredCompositionFrameBuffer();
    CreateForwardCompositionFrameBuffer();
    CreateSkyFrameBuffer();
    CreateTransparentFrameBuffer();
    CreatePostProcessFrameBuffer();
    CreateUIFrameBuffer();
}
void RenderPassManager::CreateShadowDepthRenderPass()
{
}
void RenderPassManager::CreateDeferredCompositionRenderPass()
{
}
void RenderPassManager::CreateForwardCompositionRenderPass()
{
    std::vector<vk::AttachmentDescription> attachments{
        // 0: Albedo
        vk::AttachmentDescription()
            .setFormat(mImageFactory->GetRenderTargetFormat())
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal),
        // 1: Position
        vk::AttachmentDescription()
            .setFormat(mImageFactory->GetRenderTargetFormat())
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal),
        // 2: Normal
        vk::AttachmentDescription()
            .setFormat(mImageFactory->GetRenderTargetFormat())
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal),
        // 3: Metalness/Roughness
        vk::AttachmentDescription()
            .setFormat(mImageFactory->GetRenderTargetFormat())
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal),
        // 4: AO
        vk::AttachmentDescription()
            .setFormat(mImageFactory->GetRenderTargetFormat())
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal),
        // 5: Emissive
        vk::AttachmentDescription()
            .setFormat(mImageFactory->GetRenderTargetFormat())
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal),
        // 6: Depth
        vk::AttachmentDescription()
            .setFormat(mImageFactory->GetDepthStencilFormat()) // 32位深度+8位模板存储
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eDontCare) // 深度模板不需要存储
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal),
    };
}
void RenderPassManager::CreateSkyRenderPass()
{
}
void RenderPassManager::CreateTransparentRenderPass()
{
    std::vector<vk::AttachmentDescription> attachments{
        // Render Target
        vk::AttachmentDescription()
            .setFormat(mImageFactory->GetRenderTargetFormat())
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal),
        // Depth Stencil
        vk::AttachmentDescription()
            .setFormat(mImageFactory->GetDepthStencilFormat())
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal),
    };
    std::vector<vk::AttachmentReference> colorRefs{
        vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal), // Render Target
    };
    vk::AttachmentReference depthRef{
        vk::AttachmentReference(1, vk::ImageLayout::eDepthStencilAttachmentOptimal), // Depth Stencil
    };
    std::vector<vk::SubpassDescription> subpasses{
        // 0: Render Target
        vk::SubpassDescription()
            .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
            .setColorAttachments(colorRefs)        // Render Target
            .setPDepthStencilAttachment(&depthRef) // Depth Stencil
    };
    vk::RenderPassCreateInfo renderPassCreateInfo{};
    renderPassCreateInfo.setAttachments(attachments).setSubpasses(subpasses);
    auto renderPass = mContext->GetDevice().createRenderPassUnique(renderPassCreateInfo);
    if (!renderPass)
    {
        mLogger->Error("Failed to create Transparent render pass");
    }
    mRenderPasses[RenderPassType::Transparent] = std::move(renderPass);
    mLogger->Info("Transparent render pass created successfully");
}
void RenderPassManager::CreatePostProcessRenderPass()
{
}
void RenderPassManager::CreateUIRenderPass()
{
    std::vector<vk::AttachmentDescription> attachments{
        // Render Target
        vk::AttachmentDescription()
            .setFormat(mImageFactory->GetRenderTargetFormat())
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eLoad)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
            .setFinalLayout(vk::ImageLayout::ePresentSrcKHR),
    };
    std::vector<vk::AttachmentReference> colorRefs = {
        vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal)};
    std::vector<vk::SubpassDescription> subpasses{
        vk::SubpassDescription().setPipelineBindPoint(vk::PipelineBindPoint::eGraphics).setColorAttachments(colorRefs)};
    vk::RenderPassCreateInfo renderPassCreateInfo;
    renderPassCreateInfo.setAttachments(attachments).setSubpasses(subpasses);
    auto renderPass = mContext->GetDevice().createRenderPassUnique(renderPassCreateInfo);
    if (!renderPass)
    {
        mLogger->Error("Failed to create UI render pass");
    }
    mRenderPasses[RenderPassType::UI] = std::move(renderPass);
    mLogger->Info("UI render pass created successfully");
}
void RenderPassManager::CreateShadowDepthFrameBuffer()
{
}
void RenderPassManager::CreateDeferredCompositionFrameBuffer()
{
}
void RenderPassManager::CreateForwardCompositionFrameBuffer()
{
}
void RenderPassManager::CreateSkyFrameBuffer()
{
}
void RenderPassManager::CreateTransparentFrameBuffer()
{
    mFrameBuffers[RenderPassType::Transparent].clear();
    mTransparentFrameResources.clear();

    auto swapchainImages = mContext->GetSwapchainImages();
    auto swapchainImageViews = mContext->GetSwapchainImageViews();
    auto extent = vk::Extent2D{mWidth, mHeight};
    auto renderPass = mRenderPasses[RenderPassType::Transparent].get();

    for (size_t i = 0; i < swapchainImageViews.size(); ++i)
    {
        auto translucencyFrameResource = std::make_shared<TransparentFrameResource>();
        // Render Target
        auto renderTargetImage = mImageFactory->CreateImage(ImageType::RenderTarget, vk::Extent3D(extent, 1));
        auto renderTargetImageView = mImageFactory->CreateImageView(renderTargetImage.get());
        translucencyFrameResource->renderTargetImage = std::move(renderTargetImage);
        translucencyFrameResource->renderTargetImageView = std::move(renderTargetImageView);
        // Depth Stencil
        auto depthImage = mImageFactory->CreateImage(ImageType::DepthStencil, vk::Extent3D(extent, 1));
        auto depthImageView = mImageFactory->CreateImageView(depthImage.get());
        translucencyFrameResource->depthStencilImage = std::move(depthImage);
        translucencyFrameResource->depthStencilImageView = std::move(depthImageView);
        // 保存资源
        mTransparentFrameResources.push_back(translucencyFrameResource);
        // 创建帧缓冲
        auto attachments = std::array<vk::ImageView, 2>{
            translucencyFrameResource->renderTargetImageView.get(),
            translucencyFrameResource->depthStencilImageView.get(),
        };
        vk::FramebufferCreateInfo framebufferCreateInfo;
        framebufferCreateInfo.setRenderPass(renderPass)
            .setAttachments(attachments)
            .setWidth(extent.width)
            .setHeight(extent.height)
            .setLayers(1);
        auto framebuffer = mContext->GetDevice().createFramebufferUnique(framebufferCreateInfo);
        if (!framebuffer)
        {
            mLogger->Error("Failed to create framebuffer for Transparent render pass");
        }
        mFrameBuffers[RenderPassType::Transparent].push_back(std::move(framebuffer));
        mLogger->Info("Framebuffer {} for Transparent render pass created successfully", i);
    }
}
void RenderPassManager::CreatePostProcessFrameBuffer()
{
}
void RenderPassManager::CreateUIFrameBuffer()
{
    mFrameBuffers[RenderPassType::UI].clear();
    mUIFrameResources.clear();
    auto swapchainImages = mContext->GetSwapchainImages();
    auto swapchainImageViews = mContext->GetSwapchainImageViews();
    auto extent = mContext->GetSurfaceInfo().extent;
    for (size_t i = 0; i < swapchainImageViews.size(); ++i)
    {
        auto uiFrameResource = std::make_shared<UIFrameResource>();
        // Render Target
        auto swapchainImageView = swapchainImageViews[i];
        auto swapchainImage = swapchainImages[i];
        uiFrameResource->renderTargetImage = swapchainImage;
        uiFrameResource->renderTargetImageView = swapchainImageView;
        // 保存资源
        mUIFrameResources.push_back(uiFrameResource);
        // 创建帧缓冲
        std::array<vk::ImageView, 1> attachments = {uiFrameResource->renderTargetImageView};
        vk::FramebufferCreateInfo framebufferCreateInfo;
        framebufferCreateInfo.setRenderPass(mRenderPasses[RenderPassType::UI].get())
            .setAttachments(attachments)
            .setWidth(extent.width)
            .setHeight(extent.height)
            .setLayers(1);
        auto framebuffer = mContext->GetDevice().createFramebufferUnique(framebufferCreateInfo);
        if (!framebuffer)
        {
            mLogger->Error("Failed to create framebuffer for UI render pass");
        }
        mFrameBuffers[RenderPassType::UI].push_back(std::move(framebuffer));
        mLogger->Info("Framebuffer {} for UI render pass created successfully", i);
    }
}
vk::RenderPass RenderPassManager::GetRenderPass(RenderPassType type) const
{
    auto it = mRenderPasses.find(type);
    if (it != mRenderPasses.end())
    {
        return it->second.get();
    }
    else
    {
        mLogger->Error("Render pass not found for type {}", magic_enum::enum_name(type));
        return nullptr;
    }
}
std::vector<vk::Framebuffer> RenderPassManager::GetFrameBuffer(RenderPassType type) const
{
    std::vector<vk::Framebuffer> framebuffers;
    auto it = mFrameBuffers.find(type);
    if (it != mFrameBuffers.end())
    {
        for (const auto &framebuffer : it->second)
        {
            framebuffers.push_back(framebuffer.get());
        }
    }
    else
    {
        mLogger->Error("Frame buffer not found for type {}", magic_enum::enum_name(type));
        throw std::runtime_error(std::string("Frame buffer not found for type ") +
                                 std::string(magic_enum::enum_name(type)));
    }
    return framebuffers;
}
void RenderPassManager::RecreateFrameBuffer(uint32_t width, uint32_t height)
{
    mWidth = width;
    mHeight = height;
    mContext->GetDevice().waitIdle();
    CreateShadowDepthFrameBuffer();
    CreateDeferredCompositionFrameBuffer();
    CreateForwardCompositionFrameBuffer();
    CreateSkyFrameBuffer();
    CreateTransparentFrameBuffer();
    CreatePostProcessFrameBuffer();
    // CreateUIFrameBuffer();
    mLogger->Info("Frame buffers recreated with {}x{} successfully", width, height);
}
void RenderPassManager::RecreateUIFrameBuffer()
{
    CreateUIFrameBuffer();
    mLogger->Info("UI Frame buffer recreated successfully");
}
} // namespace MEngine