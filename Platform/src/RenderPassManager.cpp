#include "RenderPassManager.hpp"
#include <array>
#include <vector>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace MEngine
{
RenderPassManager::RenderPassManager(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                                     std::shared_ptr<ImageFactory> imageFactory)
    : mLogger(logger), mContext(context), mImageFactory(imageFactory)
{
    mFrameCount = mContext->GetSwapchainImages().size();
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
    std::vector<vk::AttachmentDescription> attachments{
        // 0：Render Target: Color
        vk::AttachmentDescription()
            .setFormat(mImageFactory->GetRenderTargetFormat())
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal),
        // 1: Render Target: Albedo
        vk::AttachmentDescription()
            .setFormat(mImageFactory->GetRenderTargetFormat())
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal),
        // 2: Render Target: Position
        vk::AttachmentDescription()
            .setFormat(mImageFactory->GetRenderTargetFormat())
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal),
        // 3: Render Target: Normal
        vk::AttachmentDescription()
            .setFormat(mImageFactory->GetRenderTargetFormat())
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal),
        // 4: Render Target: Metalness/Roughness
        vk::AttachmentDescription()
            .setFormat(mImageFactory->GetRenderTargetFormat())
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal),
        // 5: Render Target: AO
        vk::AttachmentDescription()
            .setFormat(mImageFactory->GetRenderTargetFormat())
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal),
        // 6: Render Target: Emissive
        vk::AttachmentDescription()
            .setFormat(mImageFactory->GetRenderTargetFormat())
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal),
        // 7: Render Target: Depth
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
    // GBuffer pass
    std::vector<vk::AttachmentReference> gbufferColorRefs{
        vk::AttachmentReference(1, vk::ImageLayout::eColorAttachmentOptimal), // Albedo
        vk::AttachmentReference(2, vk::ImageLayout::eColorAttachmentOptimal), // Position
        vk::AttachmentReference(3, vk::ImageLayout::eColorAttachmentOptimal), // Normal
        vk::AttachmentReference(4, vk::ImageLayout::eColorAttachmentOptimal), // Metalness/Roughness
        vk::AttachmentReference(5, vk::ImageLayout::eColorAttachmentOptimal), // AO
        vk::AttachmentReference(6, vk::ImageLayout::eColorAttachmentOptimal)  // Emissive
    };
    vk::AttachmentReference depthRef(7, vk::ImageLayout::eDepthStencilAttachmentOptimal);
    vk::SubpassDescription gbufferSubpass{};
    gbufferSubpass.setColorAttachments(gbufferColorRefs)
        .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
        .setPDepthStencilAttachment(&depthRef);
    // lighting pass
    std::vector<vk::AttachmentReference> lightingColorRefs{
        vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal) // Color
    };
    std::vector<vk::AttachmentReference> lightingInputColorRefs{
        {1, vk::ImageLayout::eShaderReadOnlyOptimal}, // Albedo
        {2, vk::ImageLayout::eShaderReadOnlyOptimal}, // Position
        {3, vk::ImageLayout::eShaderReadOnlyOptimal}, // Normal
        {4, vk::ImageLayout::eShaderReadOnlyOptimal}, // MetalRoughness
        {5, vk::ImageLayout::eShaderReadOnlyOptimal}, // AO
        {6, vk::ImageLayout::eShaderReadOnlyOptimal}  // Emissive
    };
    vk::SubpassDescription lightingSubpass;
    lightingSubpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
        .setColorAttachments(lightingColorRefs)
        .setInputAttachments(lightingInputColorRefs);
    std::vector<vk::SubpassDescription> subpasses{
        gbufferSubpass,
        lightingSubpass,
    };
    // dependency
    vk::SubpassDependency gBufferSubPassToLightingSubPassDependency;
    gBufferSubPassToLightingSubPassDependency
        .setSrcSubpass(0) // Guffer
        .setDstSubpass(1) // Lighting
        .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
        .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
        .setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader)
        .setDstAccessMask(vk::AccessFlagBits::eInputAttachmentRead)
        .setDependencyFlags(vk::DependencyFlagBits::eByRegion);
    std::vector<vk::SubpassDependency> dependencies{
        gBufferSubPassToLightingSubPassDependency,
    };
    vk::RenderPassCreateInfo renderPassCreateInfo{};
    renderPassCreateInfo.setAttachments(attachments).setSubpasses(subpasses).setDependencies(dependencies);
    auto renderPass = mContext->GetDevice().createRenderPassUnique(renderPassCreateInfo);
    if (!renderPass)
    {
        mLogger->Error("Failed to create Deferred Composition render pass");
    }
    mRenderPasses[RenderPassType::DeferredComposition] = std::move(renderPass);
    mLogger->Info("Deferred Composition render pass created successfully");
}
void RenderPassManager::CreateForwardCompositionRenderPass()
{
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
    mFrameBuffers[RenderPassType::DeferredComposition].clear();
    mDeferredCompositionFrameResources.clear();
    auto extent = vk::Extent2D{mWidth, mHeight};
    auto renderPass = mRenderPasses[RenderPassType::DeferredComposition].get();
    for (int i = 0; i < mFrameCount; i++)
    {
        auto deferredCompositionFrameResource = std::make_shared<DeferredCompositionFrameResource>();
        // Render Target
        auto renderTargetImage = mImageFactory->CreateImage(ImageType::RenderTarget, vk::Extent3D(extent, 1));
        auto renderTargetImageView = mImageFactory->CreateImageView(renderTargetImage.get());
        deferredCompositionFrameResource->renderTargetImage = std::move(renderTargetImage);
        deferredCompositionFrameResource->renderTargetImageView = std::move(renderTargetImageView);
        // Albedo
        auto albedoImage = mImageFactory->CreateImage(ImageType::RenderTarget, vk::Extent3D(extent, 1));
        auto albedoImageView = mImageFactory->CreateImageView(albedoImage.get());
        deferredCompositionFrameResource->albedoImage = std::move(albedoImage);
        deferredCompositionFrameResource->albedoImageView = std::move(albedoImageView);
        // Position
        auto positionImage = mImageFactory->CreateImage(ImageType::RenderTarget, vk::Extent3D(extent, 1));
        auto positionImageView = mImageFactory->CreateImageView(positionImage.get());
        deferredCompositionFrameResource->positionImage = std::move(positionImage);
        deferredCompositionFrameResource->positionImageView = std::move(positionImageView);
        // Normal
        auto normalImage = mImageFactory->CreateImage(ImageType::RenderTarget, vk::Extent3D(extent, 1));
        auto normalImageView = mImageFactory->CreateImageView(normalImage.get());
        deferredCompositionFrameResource->normalImage = std::move(normalImage);
        deferredCompositionFrameResource->normalImageView = std::move(normalImageView);
        // Metalness/Roughness
        auto metallicRoughnessImage = mImageFactory->CreateImage(ImageType::RenderTarget, vk::Extent3D(extent, 1));
        auto metallicRoughnessImageView = mImageFactory->CreateImageView(metallicRoughnessImage.get());
        deferredCompositionFrameResource->metallicRoughnessImage = std::move(metallicRoughnessImage);
        deferredCompositionFrameResource->metallicRoughnessImageView = std::move(metallicRoughnessImageView);
        // AO
        auto aoImage = mImageFactory->CreateImage(ImageType::RenderTarget, vk::Extent3D(extent, 1));
        auto aoImageView = mImageFactory->CreateImageView(aoImage.get());
        deferredCompositionFrameResource->aoImage = std::move(aoImage);
        deferredCompositionFrameResource->aoImageView = std::move(aoImageView);
        // Emissive
        auto emissiveImage = mImageFactory->CreateImage(ImageType::RenderTarget, vk::Extent3D(extent, 1));
        auto emissiveImageView = mImageFactory->CreateImageView(emissiveImage.get());
        deferredCompositionFrameResource->emissiveImage = std::move(emissiveImage);
        deferredCompositionFrameResource->emissiveImageView = std::move(emissiveImageView);
        // Depth Stencil
        auto depthImage = mImageFactory->CreateImage(ImageType::DepthStencil, vk::Extent3D(extent, 1));
        auto depthImageView = mImageFactory->CreateImageView(depthImage.get());
        deferredCompositionFrameResource->depthStencilImage = std::move(depthImage);
        deferredCompositionFrameResource->depthStencilImageView = std::move(depthImageView);
        // 保存资源
        mDeferredCompositionFrameResources.push_back(deferredCompositionFrameResource);
        // 创建帧缓冲
        std::vector<vk::ImageView> attachments = {
            deferredCompositionFrameResource->renderTargetImageView.get(),
            deferredCompositionFrameResource->albedoImageView.get(),
            deferredCompositionFrameResource->positionImageView.get(),
            deferredCompositionFrameResource->normalImageView.get(),
            deferredCompositionFrameResource->metallicRoughnessImageView.get(),
            deferredCompositionFrameResource->aoImageView.get(),
            deferredCompositionFrameResource->emissiveImageView.get(),
            deferredCompositionFrameResource->depthStencilImageView.get(),
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
            mLogger->Error("Failed to create framebuffer for Deferred Composition render pass");
        }
        mFrameBuffers[RenderPassType::DeferredComposition].push_back(std::move(framebuffer));
        mLogger->Info("Framebuffer {} for Deferred Composition render pass created successfully", i);
    }
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