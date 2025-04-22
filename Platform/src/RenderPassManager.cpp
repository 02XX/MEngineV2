#include "RenderPassManager.hpp"

namespace MEngine
{
RenderPassManager::RenderPassManager(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                                     std::shared_ptr<IConfigure> configure, std::shared_ptr<ImageFactory> imageFactory)
    : mLogger(logger), mContext(context), mImageFactory(imageFactory), mConfigure(configure)
{
    mRenderTargetWidth = mContext->GetSurfaceInfo().extent.width;
    mRenderTargetHeight = mContext->GetSurfaceInfo().extent.height;
    mEditorRenderTargetWidth = mContext->GetSurfaceInfo().extent.width;
    mEditorRenderTargetHeight = mContext->GetSurfaceInfo().extent.height;
    CreateShadowDepthRenderPass();
    CreateDeferredCompositionRenderPass();
    CreateForwardCompositionRenderPass();
    CreateSkyRenderPass();
    CreateTransparentRenderPass();
    CreatePostProcessRenderPass();
    CreateUIRenderPass();
    CreateEditorUIRenderPass();

    CreateRenderTarget();
    CreateEditorRenderTarget();

    CreateShadowDepthFrameBuffer();
    CreateDeferredCompositionFrameBuffer();
    CreateForwardCompositionFrameBuffer();
    CreateSkyFrameBuffer();
    CreateTransparentFrameBuffer();
    CreatePostProcessFrameBuffer();
    CreateUIFrameBuffer();
    CreateEditorUIFrameBuffer();
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
        // 0：Render Target: Color
        vk::AttachmentDescription()
            .setFormat(mImageFactory->GetRenderTargetFormat())
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
            .setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal),
        // 1: Render Target: Depth
        vk::AttachmentDescription()
            .setFormat(mImageFactory->GetDepthStencilFormat()) // 32位深度+8位模板存储
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
            .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal),
    };
    // SubPass: 0 不透明物体
    std::vector<vk::AttachmentReference> colorRefs{
        vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal), // Render Target: Color
    };
    vk::AttachmentReference depthRef{
        vk::AttachmentReference(1, vk::ImageLayout::eDepthStencilAttachmentOptimal), // Render Target: Depth
    };
    // SubPass: 1 透明物体
    std::vector<vk::AttachmentReference> colorRefs1{
        vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal), // Render Target: Color
    };
    vk::AttachmentReference depthRef1{
        vk::AttachmentReference(1, vk::ImageLayout::eDepthStencilAttachmentOptimal), // Render Target: Depth
    };
    std::vector<vk::SubpassDescription> subpasses{vk::SubpassDescription()
                                                      .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
                                                      .setColorAttachments(colorRefs)
                                                      .setPDepthStencilAttachment(&depthRef),
                                                  vk::SubpassDescription()
                                                      .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
                                                      .setColorAttachments(colorRefs1)
                                                      .setPDepthStencilAttachment(&depthRef1)};
    // dependency
    std::vector<vk::SubpassDependency> dependencies{
        // subpass 0 -> subpass 1
        vk::SubpassDependency()
            .setSrcSubpass(0)
            .setDstSubpass(1)
            .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
            .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite)};
    vk::RenderPassCreateInfo renderPassCreateInfo{};
    renderPassCreateInfo.setAttachments(attachments).setSubpasses(subpasses).setDependencies(dependencies);
    auto renderPass = mContext->GetDevice().createRenderPassUnique(renderPassCreateInfo);
    if (!renderPass)
    {
        mLogger->Error("Failed to create Forward render pass");
    }
    mRenderPasses[RenderPassType::ForwardComposition] = std::move(renderPass);
    mLogger->Info("Forward render pass created successfully");
}
void RenderPassManager::CreateSkyRenderPass()
{
    std::vector<vk::AttachmentDescription> attachments{
        // Render Target
        vk::AttachmentDescription()
            .setFormat(mImageFactory->GetRenderTargetFormat())
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eLoad)
            .setStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
            .setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal),
        // depth
        // Depth Stencil
        vk::AttachmentDescription()
            .setFormat(mImageFactory->GetDepthStencilFormat())
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eLoad)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
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
        mLogger->Error("Failed to create Sky render pass");
    }
    mRenderPasses[RenderPassType::Sky] = std::move(renderPass);
    mLogger->Info("Sky render pass created successfully");
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
            .setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal),
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
            .setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal),
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
void RenderPassManager::CreateEditorUIRenderPass()
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
            .setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
            .setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal),
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
        mLogger->Error("Failed to create Editor UI render pass");
    }
    mRenderPasses[RenderPassType::EditorUI] = std::move(renderPass);
    mLogger->Info("Editor UI render pass created successfully");
}

void RenderPassManager::CreateRenderTarget()
{
    auto frameCount = mContext->GetSwapchainImages().size();
    mRenderTargets.clear();
    auto extent = vk::Extent3D(mRenderTargetWidth, mRenderTargetHeight, 1);
    for (size_t i = 0; i < frameCount; i++)
    {
        // Render Target 0: Color
        auto renderTargetColorImage = mImageFactory->CreateImage(ImageType::RenderTarget, extent);
        auto renderTargetColorImageView = mImageFactory->CreateImageView(renderTargetColorImage.get());
        // Render Target 1: Depth/Stencil
        auto renderTargetDepthImage = mImageFactory->CreateImage(ImageType::DepthStencil, extent);
        auto renderTargetDepthImageView = mImageFactory->CreateImageView(renderTargetDepthImage.get());
        // Render Target 2: Albedo
        auto renderTargetAlbedoImage = mImageFactory->CreateImage(ImageType::RenderTarget, extent);
        auto renderTargetAlbedoImageView = mImageFactory->CreateImageView(renderTargetAlbedoImage.get());
        // Render Target 3: Normal
        auto renderTargetNormalImage = mImageFactory->CreateImage(ImageType::RenderTarget, extent);
        auto renderTargetNormalImageView = mImageFactory->CreateImageView(renderTargetNormalImage.get());
        // Render Target 4: WorldPos
        auto renderTargetWorldPosImage = mImageFactory->CreateImage(ImageType::RenderTarget, extent);
        auto renderTargetWorldPosImageView = mImageFactory->CreateImageView(renderTargetWorldPosImage.get());
        // Render Target 5: MetallicRoughness
        auto renderTargetMetallicRoughnessImage = mImageFactory->CreateImage(ImageType::RenderTarget, extent);
        auto renderTargetMetallicRoughnessImageView =
            mImageFactory->CreateImageView(renderTargetMetallicRoughnessImage.get());
        // Render Target 6: Emissive
        auto renderTargetEmissiveImage = mImageFactory->CreateImage(ImageType::RenderTarget, extent);
        auto renderTargetEmissiveImageView = mImageFactory->CreateImageView(renderTargetEmissiveImage.get());
        // 保存资源
        auto renderTarget = std::make_unique<RenderTarget>();
        renderTarget->colorImage = std::move(renderTargetColorImage);
        renderTarget->colorImageView = std::move(renderTargetColorImageView);
        renderTarget->depthStencilImage = std::move(renderTargetDepthImage);
        renderTarget->depthStencilImageView = std::move(renderTargetDepthImageView);
        renderTarget->albedoImage = std::move(renderTargetAlbedoImage);
        renderTarget->albedoImageView = std::move(renderTargetAlbedoImageView);
        renderTarget->normalImage = std::move(renderTargetNormalImage);
        renderTarget->normalImageView = std::move(renderTargetNormalImageView);
        renderTarget->worldPosImage = std::move(renderTargetWorldPosImage);
        renderTarget->worldPosImageView = std::move(renderTargetWorldPosImageView);
        renderTarget->metallicRoughnessImage = std::move(renderTargetMetallicRoughnessImage);
        renderTarget->metallicRoughnessImageView = std::move(renderTargetMetallicRoughnessImageView);
        renderTarget->emissiveImage = std::move(renderTargetEmissiveImage);
        renderTarget->emissiveImageView = std::move(renderTargetEmissiveImageView);
        mRenderTargets.push_back(std::move(renderTarget));
        mLogger->Info("Render target {} created successfully", i);
    }
}
void RenderPassManager::CreateEditorRenderTarget()
{
    auto frameCount = mContext->GetSwapchainImages().size();
    mEditorRenderTargets.clear();
    auto extent = vk::Extent3D(mEditorRenderTargetWidth, mEditorRenderTargetHeight, 1);
    for (size_t i = 0; i < frameCount; i++)
    {
        // Render Target 0: Color
        auto renderTargetColorImage = mImageFactory->CreateImage(ImageType::RenderTarget, extent);
        auto renderTargetColorImageView = mImageFactory->CreateImageView(renderTargetColorImage.get());
        // 保存资源
        auto renderTarget = std::make_unique<EditorRenderTarget>();
        renderTarget->colorImage = std::move(renderTargetColorImage);
        renderTarget->colorImageView = std::move(renderTargetColorImageView);
        mEditorRenderTargets.push_back(std::move(renderTarget));
        mLogger->Info("Editor render target {} created successfully", i);
    }
}

void RenderPassManager::CreateShadowDepthFrameBuffer()
{
}
void RenderPassManager::CreateDeferredCompositionFrameBuffer()
{
}
void RenderPassManager::CreateForwardCompositionFrameBuffer()
{
    mFrameBuffers[RenderPassType::ForwardComposition].clear();
    auto frameCount = mContext->GetSwapchainImages().size();
    auto extent = vk::Extent2D{mRenderTargetWidth, mRenderTargetHeight};
    auto renderPass = mRenderPasses[RenderPassType::ForwardComposition].get();
    for (size_t i = 0; i < frameCount; i++)
    {
        // 创建帧缓冲
        std::vector<vk::ImageView> attachments{
            mRenderTargets[i]->colorImageView.get(),        // Render Target: Color
            mRenderTargets[i]->depthStencilImageView.get(), // Depth Stencil
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
            mLogger->Error("Failed to create framebuffer for Forward render pass");
        }
        mFrameBuffers[RenderPassType::ForwardComposition].push_back(std::move(framebuffer));
        mLogger->Info("Framebuffer {} for Forward render pass created successfully", i);
    }
}
void RenderPassManager::CreateSkyFrameBuffer()
{
    mFrameBuffers[RenderPassType::Sky].clear();
    auto frameCount = mContext->GetSwapchainImages().size();
    auto extent = vk::Extent2D{mRenderTargetWidth, mRenderTargetHeight};
    auto renderPass = mRenderPasses[RenderPassType::Sky].get();
    for (size_t i = 0; i < frameCount; i++)
    {
        // 创建帧缓冲
        std::vector<vk::ImageView> attachments{
            mRenderTargets[i]->colorImageView.get(),        // Render Target: Color
            mRenderTargets[i]->depthStencilImageView.get(), // Depth Stencil
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
            mLogger->Error("Failed to create framebuffer for Sky render pass");
        }
        mFrameBuffers[RenderPassType::Sky].push_back(std::move(framebuffer));
        mLogger->Info("Framebuffer {} for Sky render pass created successfully", i);
    }
}
void RenderPassManager::CreateTransparentFrameBuffer()
{
    mFrameBuffers[RenderPassType::Transparent].clear();

    auto swapchainImages = mContext->GetSwapchainImages();
    auto swapchainImageViews = mContext->GetSwapchainImageViews();
    auto extent = vk::Extent2D{mRenderTargetWidth, mRenderTargetHeight};
    auto renderPass = mRenderPasses[RenderPassType::Transparent].get();

    for (size_t i = 0; i < swapchainImageViews.size(); ++i)
    {
        // 创建帧缓冲
        std::vector<vk::ImageView> attachments{
            mRenderTargets[i]->colorImageView.get(),        // Render Target: Color
            mRenderTargets[i]->depthStencilImageView.get(), // Depth Stencil
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
}
void RenderPassManager::CreateEditorUIFrameBuffer()
{
    mFrameBuffers[RenderPassType::EditorUI].clear();
    auto extent = vk::Extent2D{mEditorRenderTargetWidth, mEditorRenderTargetHeight};
    auto swapchainImages = mContext->GetSwapchainImages();
    auto swapchainImageViews = mContext->GetSwapchainImageViews();
    auto renderPass = mRenderPasses[RenderPassType::EditorUI].get();
    for (size_t i = 0; i < swapchainImageViews.size(); ++i)
    {
        // 创建帧缓冲
        std::vector<vk::ImageView> attachments{
            mEditorRenderTargets[i]->colorImageView.get(), // Render Target: Color
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
            mLogger->Error("Failed to create framebuffer for UI render pass");
        }
        mFrameBuffers[RenderPassType::EditorUI].push_back(std::move(framebuffer));
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
void RenderPassManager::RecreateRenderTargetFrameBuffer(uint32_t width, uint32_t height)
{
    mRenderTargetWidth = width;
    mRenderTargetHeight = height;
    mContext->GetDevice().waitIdle();
    CreateRenderTarget();

    CreateShadowDepthFrameBuffer();
    CreateDeferredCompositionFrameBuffer();
    CreateForwardCompositionFrameBuffer();
    CreateSkyFrameBuffer();
    CreateTransparentFrameBuffer();
    CreatePostProcessFrameBuffer();
    CreateUIFrameBuffer();
    mLogger->Info("Render target frame buffers recreated with {}x{} successfully", width, height);
}

void RenderPassManager::RecreateEditorRenderTargetFrameBuffer(uint32_t width, uint32_t height)
{
    mEditorRenderTargetWidth = width;
    mEditorRenderTargetHeight = height;
    mContext->GetDevice().waitIdle();
    CreateEditorRenderTarget();
    CreateEditorUIFrameBuffer();
    mLogger->Info("Editor render target frame buffers recreated with {}x{} successfully", width, height);
}
} // namespace MEngine