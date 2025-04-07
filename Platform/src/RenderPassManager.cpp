#include "RenderPassManager.hpp"

namespace MEngine
{
RenderPassManager::RenderPassManager(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                                     std::shared_ptr<ImageManager> imageManager)
    : mLogger(logger), mContext(context), mImageManager(imageManager)
{
    // 创建延迟渲染的GBuffer渲染通道
    // CreateGBufferRenderPass();
    // // 创建阴影深度图渲染通道
    // CreateShadowDepthRenderPass();
    // // 创建光照渲染通道
    // CreateLightingRenderPass();
    // 创建半透明物体渲染通道
    CreateTranslucencyRenderPass();
    // // 创建后处理渲染通道
    // CreatePostProcessRenderPass();
    // // 创建天空盒渲染通道
    // CreateSkyRenderPass();
    // 创建UI渲染通道
    CreateUIRenderPass();

    RecreateFrameBuffer(mWidth, mHeight);
}
void RenderPassManager::CreateGBufferRenderPass()
{
    // 1. 创建附件
    std::array<vk::AttachmentDescription, 7> attachments;
    // 1.1 世界空间位置附件
    attachments[0]
        .setFormat(vk::Format::eR32G32B32A32Sfloat) // 高精度存储位置
        .setSamples(vk::SampleCountFlagBits::e1)
        .setLoadOp(vk::AttachmentLoadOp::eDontCare) // 渲染前清空
        .setStoreOp(vk::AttachmentStoreOp::eStore)  // 存储以便后续子流程使用
        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal); // 最终布局为着色器只读
    // 1.2 法线附件
    attachments[1]
        .setFormat(vk::Format::eR16G16B16A16Sfloat) // 16位浮点存储法线
        .setSamples(vk::SampleCountFlagBits::e1)
        .setLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
    // 1.3 Albedo附件
    attachments[2]
        .setFormat(vk::Format::eR8G8B8A8Unorm) // 8位无符号存储基础颜色
        .setSamples(vk::SampleCountFlagBits::e1)
        .setLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
    // 1.4 金属度/粗糙度附件
    attachments[3]
        .setFormat(vk::Format::eR8G8B8A8Unorm) // 8位无符号存储金属度和粗糙度
        .setSamples(vk::SampleCountFlagBits::e1)
        .setLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
    // 1.5 环境光遮蔽附件
    attachments[4]
        .setFormat(vk::Format::eR8Unorm) // 8位无符号存储环境光遮蔽
        .setSamples(vk::SampleCountFlagBits::e1)
        .setLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
    // 1.6 深度模板附件
    attachments[5]
        .setFormat(vk::Format::eD32SfloatS8Uint) // 32位深度+8位模板存储
        .setSamples(vk::SampleCountFlagBits::e1)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eDontCare) // 深度模板不需要存储
        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
    // 1.7 Swapchain附件
    attachments[6]
        .setFormat(mContext->GetSurfaceInfo().format.format) // Swapchain格式
        .setSamples(vk::SampleCountFlagBits::e1)
        .setLoadOp(vk::AttachmentLoadOp::eDontCare) // 不需要清空
        .setStoreOp(vk::AttachmentStoreOp::eStore)  // 存储以便后续使用
        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
    // 2. 创建子通道
    std::array<vk::SubpassDescription, 2> subpasses;
    // 2.1 GBuffer子通道
    std::array<vk::AttachmentReference, 5> gBufferColorRefs = {
        vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal), // 位置
        vk::AttachmentReference(1, vk::ImageLayout::eColorAttachmentOptimal), // 法线
        vk::AttachmentReference(2, vk::ImageLayout::eColorAttachmentOptimal), // Albedo
        vk::AttachmentReference(3, vk::ImageLayout::eColorAttachmentOptimal), // 金属度/粗糙度
        vk::AttachmentReference(4, vk::ImageLayout::eColorAttachmentOptimal)  // 环境光遮蔽
    };
    vk::AttachmentReference depthRef(5, vk::ImageLayout::eDepthStencilAttachmentOptimal);
    subpasses[0]
        .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics) // 图形管线绑定点
        .setColorAttachments(gBufferColorRefs)                  // 颜色附件引用
        .setPDepthStencilAttachment(&depthRef)                  // 深度模板附件引用
        .setInputAttachments(nullptr)                           // 输入附件引用（无）
        .setPreserveAttachments(nullptr)                        // 保留附件（无）
        .setResolveAttachments(nullptr);                        // 解析附件（无）
    // 2.2 Lighting子通道
    std::array<vk::AttachmentReference, 5> lightingInputRefs = {
        vk::AttachmentReference(0, vk::ImageLayout::eShaderReadOnlyOptimal), // 位置
        vk::AttachmentReference(1, vk::ImageLayout::eShaderReadOnlyOptimal), // 法线
        vk::AttachmentReference(2, vk::ImageLayout::eShaderReadOnlyOptimal), // Albedo
        vk::AttachmentReference(3, vk::ImageLayout::eShaderReadOnlyOptimal), // 金属度/粗糙度
        vk::AttachmentReference(4, vk::ImageLayout::eShaderReadOnlyOptimal)  // 环境光遮蔽
    };
    // Swapchain color attachment
    vk::AttachmentReference swapchainRef(6, vk::ImageLayout::eColorAttachmentOptimal);
    subpasses[1]
        .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics) // 图形管线绑定点
        .setColorAttachments(swapchainRef)                      // 颜色附件引用（Swapchain）
        .setInputAttachments(lightingInputRefs);                // 输入附件引用

    // 3. 定义子通道依赖关系
    std::array<vk::SubpassDependency, 1> dependencies;
    // subpass0->subpass1
    dependencies[1]
        .setSrcSubpass(0)
        .setDstSubpass(1)
        .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
        .setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader)
        .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
        .setDstAccessMask(vk::AccessFlagBits::eInputAttachmentRead)
        .setDependencyFlags(vk::DependencyFlagBits::eByRegion);
    // subpass1->外部
    dependencies[2]
        .setSrcSubpass(1)
        .setDstSubpass(vk::SubpassExternal)
        .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
        .setDstStageMask(vk::PipelineStageFlagBits::eBottomOfPipe)
        .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
        .setDstAccessMask(vk::AccessFlagBits::eNone)
        .setDependencyFlags(vk::DependencyFlagBits::eByRegion);
    // 4. 创建渲染通道
    vk::RenderPassCreateInfo renderPassCreateInfo;
    renderPassCreateInfo
        .setAttachments(attachments)    // 附件描述
        .setSubpasses(subpasses)        // 子通道描述
        .setDependencies(dependencies); // 依赖关系描述
    auto renderPass = mContext->GetDevice().createRenderPassUnique(renderPassCreateInfo);
    if (!renderPass)
    {
        mLogger->Error("Failed to create GBuffer render pass");
    }
    mRenderPasses[RenderPassType::Deffer] = std::move(renderPass);
    mLogger->Debug("Deffer render pass created successfully");
}
void RenderPassManager::CreateShadowDepthRenderPass()
{
}
void RenderPassManager::CreateLightingRenderPass()
{
}
void RenderPassManager::CreateTranslucencyRenderPass()
{
    // 1. 创建颜色附件
    std::array<vk::AttachmentDescription, 2> attachments{
        vk::AttachmentDescription()
            .setFormat(mContext->GetSurfaceInfo().format.format) // Swapchain格式
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal),
        vk::AttachmentDescription()
            .setFormat(vk::Format::eD32SfloatS8Uint) // 32位深度+8位模板存储
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal) // 深度模板
    };

    // 2. 创建子通道
    std::array<vk::AttachmentReference, 1> colorRefs = {
        vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal) // Swapchain
    };
    vk::AttachmentReference depthRef(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);
    std::array<vk::SubpassDescription, 1> subpasses{vk::SubpassDescription()
                                                        .setColorAttachments(colorRefs)
                                                        .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
                                                        .setPDepthStencilAttachment(&depthRef)};
    // 4. 创建渲染通道
    vk::RenderPassCreateInfo renderPassCreateInfo{};
    renderPassCreateInfo.setAttachments(attachments).setSubpasses(subpasses).setDependencies({});
    auto renderPass = mContext->GetDevice().createRenderPassUnique(renderPassCreateInfo);
    if (!renderPass)
    {
        mLogger->Error("Failed to create translucency render pass");
    }
    mRenderPasses[RenderPassType::Translucency] = std::move(renderPass);
    mLogger->Debug("Translucency render pass created successfully");
}
void RenderPassManager::CreatePostProcessRenderPass()
{
}
void RenderPassManager::CreateSkyRenderPass()
{
}
void RenderPassManager::CreateUIRenderPass()
{
    // 1. 创建附件
    std::array<vk::AttachmentDescription, 1> attachments{
        vk::AttachmentDescription()
            .setFormat(mContext->GetSurfaceInfo().format.format) // Swapchain格式
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eLoad)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
            .setFinalLayout(vk::ImageLayout::ePresentSrcKHR)};

    // 2. 创建子通道
    std::array<vk::SubpassDescription, 1> subpasses;
    std::array<vk::AttachmentReference, 1> colorRefs = {
        vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal) // Swapchain
    };
    subpasses[0]
        .setColorAttachments(colorRefs)                          // 颜色附件引用
        .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics); // 图形管线绑定点

    // 4. 创建渲染通道
    vk::RenderPassCreateInfo renderPassCreateInfo;
    renderPassCreateInfo
        .setAttachments(attachments) // 附件描述
        .setSubpasses(subpasses);    // 子通道描述
    auto renderPass = mContext->GetDevice().createRenderPassUnique(renderPassCreateInfo);
    if (!renderPass)
    {
        mLogger->Error("Failed to create UI render pass");
    }
    mRenderPasses[RenderPassType::UI] = std::move(renderPass);
    mLogger->Debug("UI render pass created successfully");
}

void RenderPassManager::CreateDefferFrameBuffer()
{
    mFrameBuffers[RenderPassType::Deffer].clear();
    mDefferFrameResources.clear();

    auto swapchainImages = mContext->GetSwapchainImages();
    auto swapchainImageViews = mContext->GetSwapchainImageViews();
    mDefferFrameResources.resize(swapchainImageViews.size());
    auto extent = vk::Extent2D{mWidth, mHeight};
    for (size_t i = 0; i < swapchainImageViews.size(); ++i)
    {
        auto defferFrameResource = DefferFrameResource{};
        // 1. 创建世界空间位置图像和视图
        auto positionImage = mImageManager->CreateUniqueTexture2D(extent, vk::Format::eR32G32B32A32Sfloat);
        auto positionImageView = mImageManager->CreateImageView(
            positionImage->GetImage(), vk::Format::eR32G32B32A32Sfloat, vk::ComponentMapping{},
            vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
        defferFrameResource.positionImage = std::move(positionImage);
        defferFrameResource.positionImageView = std::move(positionImageView);
        // 2. 创建法线图像和视图
        auto normalImage = mImageManager->CreateUniqueTexture2D(extent, vk::Format::eR16G16B16A16Sfloat);
        auto normalImageView = mImageManager->CreateImageView(
            normalImage->GetImage(), vk::Format::eR16G16B16A16Sfloat, vk::ComponentMapping{},
            vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
        defferFrameResource.normalImage = std::move(normalImage);
        defferFrameResource.normalImageView = std::move(normalImageView);
        // 3. 创建Albedo图像和视图
        auto albedoImage = mImageManager->CreateUniqueTexture2D(extent, vk::Format::eR8G8B8A8Unorm);
        auto albedoImageView =
            mImageManager->CreateImageView(albedoImage->GetImage(), vk::Format::eR8G8B8A8Unorm, vk::ComponentMapping{},
                                           vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
        defferFrameResource.albedoImage = std::move(albedoImage);
        defferFrameResource.albedoImageView = std::move(albedoImageView);
        // 4. 创建金属度/粗糙度图像和视图
        auto metalRoughImage = mImageManager->CreateUniqueTexture2D(extent, vk::Format::eR8G8B8A8Unorm);
        auto metalRoughImageView = mImageManager->CreateImageView(
            metalRoughImage->GetImage(), vk::Format::eR8G8B8A8Unorm, vk::ComponentMapping{},
            vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
        defferFrameResource.metalRoughImage = std::move(metalRoughImage);
        defferFrameResource.metalRoughImageView = std::move(metalRoughImageView);
        // 5. 创建环境光遮蔽图像和视图
        auto aoImage = mImageManager->CreateUniqueTexture2D(extent, vk::Format::eR8Unorm);
        auto aoImageView =
            mImageManager->CreateImageView(aoImage->GetImage(), vk::Format::eR8Unorm, vk::ComponentMapping{},
                                           vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
        defferFrameResource.aoImage = std::move(aoImage);
        defferFrameResource.aoImageView = std::move(aoImageView);
        // 6. 创建深度模板图像和视图
        auto extent = mContext->GetSurfaceInfo().extent;
        auto depthImage = mImageManager->CreateUniqueDepthStencil(extent);
        auto depthImageView = mImageManager->CreateImageView(
            depthImage->GetImage(), vk::Format::eD32SfloatS8Uint, vk::ComponentMapping{},
            vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, 1, 0, 1});
        defferFrameResource.depthStencilImage = std::move(depthImage);
        defferFrameResource.depthStencilImageView = std::move(depthImageView);
        // 7. 创建render image和视图
        auto renderImage =
            mImageManager->CreateUniqueTexture2D(extent, mContext->GetSurfaceInfo().format.format, 1, nullptr);
        auto rederImageView = mImageManager->CreateImageView(
            renderImage->GetImage(), mContext->GetSurfaceInfo().format.format, vk::ComponentMapping{},
            vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
        defferFrameResource.renderImage = std::move(renderImage);
        defferFrameResource.renderImageView = std::move(rederImageView);

        // 保存资源
        mDefferFrameResources.push_back(std::move(defferFrameResource));
        // 创建帧缓冲
        std::array<vk::ImageView, 7> attachments = {mDefferFrameResources.back().positionImageView.get(),
                                                    mDefferFrameResources.back().normalImageView.get(),
                                                    mDefferFrameResources.back().albedoImageView.get(),
                                                    mDefferFrameResources.back().metalRoughImageView.get(),
                                                    mDefferFrameResources.back().aoImageView.get(),
                                                    mDefferFrameResources.back().depthStencilImageView.get(),
                                                    mDefferFrameResources.back().renderImageView.get()};
        vk::FramebufferCreateInfo framebufferCreateInfo;
        framebufferCreateInfo.setRenderPass(mRenderPasses[RenderPassType::Deffer].get())
            .setAttachments(attachments)
            .setWidth(extent.width)
            .setHeight(extent.height)
            .setLayers(1);
        auto framebuffer = mContext->GetDevice().createFramebufferUnique(framebufferCreateInfo);
        if (!framebuffer)
        {
            mLogger->Error("Failed to create framebuffer for Deffer render pass");
        }
        mFrameBuffers[RenderPassType::Deffer].push_back(std::move(framebuffer));
        mLogger->Debug("Framebuffer {} for Deffer render pass created successfully", i);
    }
    mLogger->Debug("{} Deffer frame buffers created successfully", mFrameBuffers[RenderPassType::Deffer].size());
}
void RenderPassManager::CreateShadowDepthFrameBuffer()
{
}
void RenderPassManager::CreateLightingFrameBuffer()
{
}
void RenderPassManager::CreateTranslucencyFrameBuffer()
{
    mFrameBuffers[RenderPassType::Translucency].clear();
    mTranslucencyFrameResources.clear();

    auto swapchainImages = mContext->GetSwapchainImages();
    auto swapchainImageViews = mContext->GetSwapchainImageViews();
    auto extent = vk::Extent2D{mWidth, mHeight};
    auto renderPass = mRenderPasses[RenderPassType::Translucency].get();

    mTranslucencyFrameResources.resize(swapchainImageViews.size());
    for (size_t i = 0; i < swapchainImageViews.size(); ++i)
    {
        // 创建render image和视图
        auto renderImage =
            mImageManager->CreateUniqueTexture2D(extent, mContext->GetSurfaceInfo().format.format, 1, nullptr);
        auto rederImageView = mImageManager->CreateImageView(
            renderImage->GetImage(), mContext->GetSurfaceInfo().format.format, vk::ComponentMapping{},
            vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
        mTranslucencyFrameResources[i].renderImage = std::move(renderImage);
        mTranslucencyFrameResources[i].renderImageView = std::move(rederImageView);
        // 创建深度模板图像和视图
        auto depthImage = mImageManager->CreateUniqueDepthStencil(extent);
        auto depthImageView = mImageManager->CreateImageView(
            depthImage->GetImage(), vk::Format::eD32SfloatS8Uint, vk::ComponentMapping{},
            vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, 1, 0, 1});
        mTranslucencyFrameResources[i].depthStencilImage = std::move(depthImage);
        mTranslucencyFrameResources[i].depthStencilImageView = std::move(depthImageView);

        auto attachments = std::array<vk::ImageView, 2>{
            mTranslucencyFrameResources[i].renderImageView.get(),       // Swapchain图像视图
            mTranslucencyFrameResources[i].depthStencilImageView.get(), // 深度模板图像视图
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
            mLogger->Error("Failed to create framebuffer for Translucency render pass");
        }
        mFrameBuffers[RenderPassType::Translucency].push_back(std::move(framebuffer));
        mLogger->Debug("Framebuffer {} for Translucency render pass created successfully", i);
    }
    mLogger->Debug("{} Translucency frame buffers created successfully",
                   mFrameBuffers[RenderPassType::Translucency].size());
}
void RenderPassManager::CreatePostProcessFrameBuffer()
{
}
void RenderPassManager::CreateSkyFrameBuffer()
{
}
void RenderPassManager::CreateUIFrameBuffer()
{
    auto swapchainImages = mContext->GetSwapchainImages();
    auto swapchainImageViews = mContext->GetSwapchainImageViews();
    auto extent = mContext->GetSurfaceInfo().extent;
    for (size_t i = 0; i < swapchainImageViews.size(); ++i)
    {
        auto uiFrameResource = UIFrameResource{};
        // 1. 创建Swapchain图像和视图
        auto swapchainImageView = swapchainImageViews[i];
        auto swapchainImage = swapchainImages[i];
        uiFrameResource.swapchainImage = swapchainImage;
        uiFrameResource.swapchainImageView = swapchainImageView;
        // 保存资源
        mUIFrameResources.push_back(std::move(uiFrameResource));
        // 创建帧缓冲
        std::array<vk::ImageView, 1> attachments = {mUIFrameResources.back().swapchainImageView};
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
        mLogger->Debug("Framebuffer {} for UI render pass created successfully", i);
    }

    mLogger->Debug("{} UI frame buffers created successfully", mFrameBuffers[RenderPassType::UI].size());
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
vk::Framebuffer RenderPassManager::GetFrameBuffer(RenderPassType type, uint32_t index) const
{
    auto it = mFrameBuffers.find(type);
    if (it != mFrameBuffers.end() && index < it->second.size())
    {
        return it->second[index].get();
    }
    else
    {
        mLogger->Error("Framebuffer not found for type {} and index {}", magic_enum::enum_name(type), index);
        return nullptr;
    }
}
void RenderPassManager::RecreateFrameBuffer(uint32_t width, uint32_t height)
{
    mWidth = width;
    mHeight = height;
    // 创建延迟渲染的GBuffer帧缓冲
    // CreateDefferFrameBuffer();
    // // 创建阴影深度图帧缓冲
    // CreateShadowDepthFrameBuffer();
    // // 创建光照帧缓冲
    // CreateLightingFrameBuffer();
    // 创建半透明物体帧缓冲
    CreateTranslucencyFrameBuffer();
    // // 创建后处理帧缓冲
    // CreatePostProcessFrameBuffer();
    // // 创建天空盒帧缓冲
    // CreateSkyFrameBuffer();
    // 创建UI帧缓冲
    CreateUIFrameBuffer();
}
} // namespace MEngine