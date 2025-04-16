#include "System/RenderSystem.hpp"
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace MEngine
{

RenderSystem::RenderSystem(
    std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context, std::shared_ptr<IConfigure> configure,
    std::shared_ptr<entt::registry> registry, std::shared_ptr<RenderPassManager> renderPassManager,
    std::shared_ptr<PipelineLayoutManager> pipelineLayoutManager, std::shared_ptr<PipelineManager> pipelineManager,
    std::shared_ptr<CommandBufferManager> commandBufferManager,
    std::shared_ptr<SyncPrimitiveManager> syncPrimitiveManager, std::shared_ptr<DescriptorManager> descriptorManager,
    std::shared_ptr<SamplerManager> samplerManager, std::shared_ptr<BufferFactory> bufferFactory,
    std::shared_ptr<ImageFactory> imageFactory, std::shared_ptr<IWindow> window, std::shared_ptr<UI> ui)
    : System(logger, context, configure, registry), mRenderPassManager(renderPassManager),
      mPipelineLayoutManager(pipelineLayoutManager), mPipelineManager(pipelineManager),
      mCommandBufferManager(commandBufferManager), mSyncPrimitiveManager(syncPrimitiveManager),
      mDescriptorManager(descriptorManager), mSamplerManager(samplerManager), mBufferFactory(bufferFactory),
      mImageFactory(imageFactory), mWindow(window), mUI(ui)
{
}
void RenderSystem::Init()
{
    mFrameCount = mContext->GetSwapchainImageViews().size();
    mFrameIndex = 0;
    // command buffer
    mGraphicCommandBuffers =
        mCommandBufferManager->CreatePrimaryCommandBuffers(CommandBufferType::Graphic, mFrameCount);
    // mSecondaryCommandBuffers = std::vector<std::vector<vk::UniqueCommandBuffer>>(mFrameCount);
    // fence/semaphore
    for (size_t i = 0; i < mFrameCount; ++i)
    {
        auto imageAvailableSemaphore = mSyncPrimitiveManager->CreateUniqueSemaphore();
        auto renderFinishedSemaphores = mSyncPrimitiveManager->CreateUniqueSemaphore();
        auto inFlightFence = mSyncPrimitiveManager->CreateFence(vk::FenceCreateFlagBits::eSignaled);
        mImageAvailableSemaphores.push_back(std::move(imageAvailableSemaphore));
        mRenderFinishedSemaphores.push_back(std::move(renderFinishedSemaphores));
        mInFlightFences.push_back(std::move(inFlightFence));
    }
    mUIUpdateFence = mSyncPrimitiveManager->CreateFence(vk::FenceCreateFlagBits::eSignaled);
    // Uniform Buffer
    mVPUBO = mBufferFactory->CreateBuffer(BufferType::Uniform, sizeof(VPUniform));
    auto globalDescriptorSetLayout = mPipelineLayoutManager->GetGlobalDescriptorSetLayout();
    for (uint32_t i = 0; i < mFrameCount; ++i)
    {
        auto sets = mDescriptorManager->AllocateUniqueDescriptorSet({globalDescriptorSetLayout});
        mGlobalDescriptorSets.push_back(std::move(sets[0]));
    }
    mLastFrameImages.resize(mFrameCount);
    mLastFrameImageViews.resize(mFrameCount);
    mWindow->SetEventCallback([this](const void *event) { mUI->ProcessEvent(static_cast<const SDL_Event *>(event)); });
    TransitionSwapchainImageLayout();

    mIsInit = true;
    mLogger->Info("RenderSystem Initialized");
}

RenderSystem::~RenderSystem()
{
    if (!mIsShutdown)
        Shutdown();
}
void RenderSystem::Shutdown()
{

    mContext->GetDevice().waitIdle();
    mIsShutdown = true;
    mLogger->Info("RenderSystem Shutdown");
}
void RenderSystem::TransitionSwapchainImageLayout()
{
    auto fence = mSyncPrimitiveManager->CreateFence();
    auto swapchainImages = mContext->GetSwapchainImages();
    mGraphicCommandBuffers[mFrameIndex]->begin(vk::CommandBufferBeginInfo{});
    for (auto swapchainImage : swapchainImages)
    {
        vk::ImageMemoryBarrier swapchainImageMemoryBarrier;
        swapchainImageMemoryBarrier.setImage(swapchainImage)
            .setOldLayout(vk::ImageLayout::eUndefined)
            .setNewLayout(vk::ImageLayout::ePresentSrcKHR)
            .setSrcAccessMask(vk::AccessFlagBits::eNone)
            .setDstAccessMask(vk::AccessFlagBits::eMemoryRead)
            .setSubresourceRange(vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
        mGraphicCommandBuffers[mFrameIndex]->pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe,
                                                             vk::PipelineStageFlagBits::eBottomOfPipe, {}, {}, {},
                                                             swapchainImageMemoryBarrier);
    }
    mGraphicCommandBuffers[mFrameIndex]->end();
    vk::SubmitInfo submitInfo;
    submitInfo.setCommandBuffers({mGraphicCommandBuffers[mFrameIndex].get()});
    mContext->SubmitToGraphicQueue({submitInfo}, fence.get());
    auto result = mContext->GetDevice().waitForFences({fence.get()}, VK_TRUE, 1000000000); // 1s
    if (result != vk::Result::eSuccess)
    {
        mLogger->Error("Failed to wait fence for transition swapchain image layout");
        throw std::runtime_error("Failed to transition SwapchainImageLayout");
    }
}

void RenderSystem::CollectRenderEntities()
{
    mRenderEntities.clear();
    auto entities = mRegistry->view<MaterialComponent, MeshComponent>();
    for (auto entity : entities)
    {
        auto &material = mRegistry->get<MaterialComponent>(entity);
        auto &mesh = entities.get<MeshComponent>(entity);
        mRenderEntities[material.material->GetRenderType()].push_back(entity);
    }
}
void RenderSystem::CollectMainCamera()
{
    auto entities = mRegistry->view<CameraComponent>();
    for (auto entity : entities)
    {
        auto &camera = entities.get<CameraComponent>(entity);
        if (camera.isMainCamera)
        {
            mMainCameraEntity = entity;
            // 设置Uniform Buffer
            mVPUniform.view = camera.viewMatrix;
            mVPUniform.projection = camera.projectionMatrix;
            memcpy(mVPUBO->GetAllocationInfo().pMappedData, &mVPUniform, sizeof(mVPUniform));
            break;
        }
    }
}
void RenderSystem::Tick(float deltaTime)
{
    // TickRotationMatrix();
    Prepare(); // Prepare
    // mUI->RenderUI();
    CollectRenderEntities(); // Collect same material render entities
    CollectMainCamera();
    // RenderShadowDepthPass();  // Shadow pass
    // RenderMainPass();       // Deffer pass
    // RenderSkyPass();          // Sky pass
    RenderTranslucencyPass(); // Translucency pass
    // RenderPostProcessPass();  // Post process pass
    // RenderUIPass(deltaTime); // UI pass
    Present(); // Present
}

void RenderSystem::Prepare()
{
    auto result = mContext->GetDevice().waitForFences({mInFlightFences[mFrameIndex].get()}, VK_TRUE,
                                                      1000000000); // 1s
    if (result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to wait fence");
    }
    mContext->GetDevice().resetFences({mInFlightFences[mFrameIndex].get()});
    mGraphicCommandBuffers[mFrameIndex]->reset();
    auto resultValue = mContext->GetDevice().acquireNextImageKHR(mContext->GetSwapchain(), 1000000000,
                                                                 mImageAvailableSemaphores[mFrameIndex].get(), nullptr);
    if (resultValue.result == vk::Result::eErrorOutOfDateKHR)
    {
        mLogger->Error("Swapchain out of date, recreating swapchain");
        mContext->GetDevice().waitIdle();
        mContext->RecreateSwapchain();
        auto width = mContext->GetSurfaceInfo().extent.width;
        auto height = mContext->GetSurfaceInfo().extent.height;
        mRenderPassManager->RecreateFrameBuffer(width, height);
        TransitionSwapchainImageLayout();
    }
    else if (resultValue.result != vk::Result::eSuccess && resultValue.result != vk::Result::eSuboptimalKHR)
    {
        throw std::runtime_error("Failed to acquire next image");
    }
    mImageIndex = resultValue.value;
    //  ReCreateFrameBuffer
    // if (mUI->IsSceneViewPortChanged())
    // {
    //     auto width = mUI->GetSceneWidth();
    //     auto height = mUI->GetSceneHeight();

    // }
    mDescriptorManager->UpdateUniformDescriptorSet({mVPUBO.get()}, 0, mGlobalDescriptorSets[mFrameIndex].get());
    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    mGraphicCommandBuffers[mFrameIndex]->begin(beginInfo);
}
void RenderSystem::RenderShadowDepthPass()
{
}
void RenderSystem::RenderDeferred()
{
}
void RenderSystem::RenderForward()
{
    auto extent = mRenderPassManager->GetExtent();
    vk::RenderPassBeginInfo renderPassBeginInfo;
    std::array<vk::ClearValue, 8> clearValues;
    clearValues[0].color = vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}); // 附件0: Color
    clearValues[1].color = vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}); // 附件1: 位置
    clearValues[2].color = vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}); // 附件2: 法线
    clearValues[3].color = vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}); // 附件3: Albedo
    clearValues[4].color = vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}); // 附件4: 金属/粗糙度
    clearValues[5].color = vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}); // 附件5: AO
    clearValues[6].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);                        // 附件6: 深度
    auto defferFrameBuffers = mRenderPassManager->GetFrameBuffer(RenderPassType::ForwardComposition);
    renderPassBeginInfo.setRenderPass(mRenderPassManager->GetRenderPass(RenderPassType::ForwardComposition))
        .setFramebuffer(defferFrameBuffers[mImageIndex])
        .setRenderArea(vk::Rect2D({0, 0}, extent))
        .setClearValues(clearValues);
    mGraphicCommandBuffers[mFrameIndex]->beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

    mGraphicCommandBuffers[mFrameIndex]->nextSubpass(vk::SubpassContents::eInline);
    mGraphicCommandBuffers[mFrameIndex]->endRenderPass();
}
void RenderSystem::RenderTranslucencyPass()
{
    auto transparentFrameResources = mRenderPassManager->GetTransparentFrameResource();
    auto extent = transparentFrameResources[mImageIndex]->renderTargetImage->GetExtent();
    // PBR
    auto forwardTransparentPBREntities = mRenderEntities[RenderType::ForwardTransparentPBR];
    auto pipelineLayout = mPipelineLayoutManager->GetPipelineLayout(PipelineLayoutType::PBR);
    auto pipeline = mPipelineManager->GetPipeline(PipelineType::ForwardTransparentPBR);
    auto renderPass = mRenderPassManager->GetRenderPass(RenderPassType::Transparent);
    auto transparentFrameBuffers = mRenderPassManager->GetFrameBuffer(RenderPassType::Transparent);
    vk::RenderPassBeginInfo renderPassBeginInfo;
    std::array<vk::ClearValue, 2> clearValues{
        vk::ClearValue(std::array<float, 4>{0.1f, 0.1f, 0.1f, 1.0f}), // 附件0: Render Target
        vk::ClearDepthStencilValue(1.0f, 0)                           // 附件1: Depth Stencil
    };
    renderPassBeginInfo.setRenderPass(renderPass)
        .setFramebuffer(transparentFrameBuffers[mImageIndex])
        .setRenderArea(vk::Rect2D({0, 0}, vk::Extent2D(extent.width, extent.height)))
        .setClearValues(clearValues);
    mGraphicCommandBuffers[mFrameIndex]->beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
    {
        // viewport
        vk::Viewport viewport;
        viewport.setX(0.0f)
            .setY(0.0f)
            .setWidth(static_cast<float>(extent.width))
            .setHeight(static_cast<float>(extent.height))
            .setMinDepth(0.0f)
            .setMaxDepth(1.0f);
        mGraphicCommandBuffers[mFrameIndex]->setViewport(0, viewport);
        // scissor
        vk::Rect2D scissor;
        scissor.setOffset({0, 0}).setExtent(vk::Extent2D(extent.width, extent.height));
        mGraphicCommandBuffers[mFrameIndex]->setScissor(0, scissor);
        // 1. 绑定管线
        mGraphicCommandBuffers[mFrameIndex]->bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

        for (auto entity : forwardTransparentPBREntities)
        {
            auto &material = mRegistry->get<MaterialComponent>(entity);
            auto &mesh = mRegistry->get<MeshComponent>(entity);
            auto &transform = mRegistry->get<TransformComponent>(entity);
            // 1. 绑定push constant
            mGraphicCommandBuffers[mFrameIndex]->pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0,
                                                               sizeof(glm::mat4x4), &transform.modelMatrix);
            // 2. 绑定Global描述符集
            mGraphicCommandBuffers[mFrameIndex]->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0,
                                                                    mGlobalDescriptorSets[mFrameIndex].get(), {});
            // 3. 绑定材质描述符集
            auto materialDescriptorSet = material.material->GetDescriptorSet();
            mGraphicCommandBuffers[mFrameIndex]->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 1,
                                                                    materialDescriptorSet, {});
            //  4. 绑定顶点缓冲区
            auto vertexBuffer = mesh.mesh->GetVertexBuffer();
            mGraphicCommandBuffers[mFrameIndex]->bindVertexBuffers(0, vertexBuffer, {0});
            // 5. 绑定索引缓冲区
            auto indexBuffer = mesh.mesh->GetIndexBuffer();
            mGraphicCommandBuffers[mFrameIndex]->bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint32);
            // 6. 绘制
            mGraphicCommandBuffers[mFrameIndex]->drawIndexed(mesh.mesh->GetIndexCount(), 1, 0, 0, 0);
        }
    }
    // Phong

    mLastFrameImages[mImageIndex] = &transparentFrameResources[mImageIndex]->renderTargetImage;
    mLastFrameImageViews[mImageIndex] = transparentFrameResources[mImageIndex]->renderTargetImageView.get();
    mGraphicCommandBuffers[mFrameIndex]->endRenderPass();
}
void RenderSystem::RenderPostProcessPass()
{
}
void RenderSystem::RenderSkyPass()
{
}
void RenderSystem::RenderUIPass(float deltaTime)
{
    vk::ImageMemoryBarrier lastFrameImageBarrier;
    lastFrameImageBarrier.setImage(mLastFrameImages[mImageIndex]->get()->GetHandle())
        .setOldLayout(vk::ImageLayout::eColorAttachmentOptimal)
        .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
        .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
        .setDstAccessMask(vk::AccessFlagBits::eShaderRead)
        .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
    mGraphicCommandBuffers[mFrameIndex]->pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                                         vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {},
                                                         lastFrameImageBarrier);
    mUI->SetSceneViewPort(mLastFrameImageViews);
    mUI->SetImageIndex(mImageIndex);
    auto queueFamilyIndices = mContext->GetQueueFamilyIndicates();
    vk::ClearValue clearValue(std::array<float, 4>{0.1f, 0.1f, 0.1f, 1.0f});
    vk::RenderPassBeginInfo renderPassBeginInfo;
    auto uiframeBuffers = mRenderPassManager->GetFrameBuffer(RenderPassType::UI);
    auto renderPass = mRenderPassManager->GetRenderPass(RenderPassType::UI);
    auto &uiFrameResources = mRenderPassManager->GetUIFrameResource();
    auto extent = uiFrameResources[mImageIndex]->renderTargetImage->GetExtent();
    mLastFrameImages[mImageIndex] = &uiFrameResources[mImageIndex]->renderTargetImage;
    // 渲染前的布局转换
    vk::ImageMemoryBarrier preRenderBarrier;
    preRenderBarrier.setImage(uiFrameResources[mImageIndex]->renderTargetImage->GetHandle())
        .setOldLayout(vk::ImageLayout::eUndefined)
        .setNewLayout(vk::ImageLayout::eColorAttachmentOptimal)
        .setSrcQueueFamilyIndex(queueFamilyIndices.graphicsFamily.value())
        .setDstQueueFamilyIndex(queueFamilyIndices.graphicsFamily.value())
        .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
        .setSrcAccessMask(vk::AccessFlagBits::eMemoryRead)
        .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

    mGraphicCommandBuffers[mFrameIndex]->pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                                         vk::PipelineStageFlagBits::eColorAttachmentOutput, {}, {}, {},
                                                         preRenderBarrier);

    // 开始渲染
    renderPassBeginInfo.setRenderPass(renderPass)
        .setFramebuffer(uiframeBuffers[mImageIndex])
        .setRenderArea(vk::Rect2D({0, 0}, vk::Extent2D(extent.width, extent.height)))
        .setClearValues(clearValue);

    mGraphicCommandBuffers[mFrameIndex]->beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
    mUI->RecordUICommandBuffer(mGraphicCommandBuffers[mFrameIndex].get());
    mGraphicCommandBuffers[mFrameIndex]->endRenderPass();
}
void RenderSystem::Present()
{
    // 渲染完成的图像转换为可传输布局
    vk::ImageMemoryBarrier presentRenderBarrier;
    presentRenderBarrier.setImage(mLastFrameImages[mImageIndex]->get()->GetHandle())
        .setOldLayout(vk::ImageLayout::eColorAttachmentOptimal)
        .setNewLayout(vk::ImageLayout::eTransferSrcOptimal)
        .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
        .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
        .setDstAccessMask(vk::AccessFlagBits::eTransferRead);
    mGraphicCommandBuffers[mFrameIndex]->pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                                         vk::PipelineStageFlagBits::eTransfer, {}, {}, {},
                                                         presentRenderBarrier);
    // swapchain图像转换为可传输布局
    vk::ImageMemoryBarrier swapchainBarrier;
    swapchainBarrier.setImage(mContext->GetSwapchainImages()[mImageIndex])
        .setOldLayout(vk::ImageLayout::ePresentSrcKHR)
        .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
        .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
        .setSrcAccessMask(vk::AccessFlagBits::eMemoryRead)
        .setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
    mGraphicCommandBuffers[mFrameIndex]->pipelineBarrier(
        vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, swapchainBarrier);

    // 拷贝渲染完成的图像到交换链图像
    // 计算偏移量
    auto swapchainExtent = mContext->GetSurfaceInfo().extent;
    auto renderTargetExtent = mLastFrameImages[mFrameIndex]->get()->GetExtent();
    // auto offsetX = (swapchainExtent.width - renderTargetExtent.width) / 2;
    // auto offsetY = (swapchainExtent.height - renderTargetExtent.height) / 2;
    vk::ImageCopy imageCopy;
    imageCopy.setSrcSubresource(vk::ImageSubresourceLayers{vk::ImageAspectFlagBits::eColor, 0, 0, 1})
        .setSrcOffset({0, 0, 0})
        .setDstSubresource(vk::ImageSubresourceLayers{vk::ImageAspectFlagBits::eColor, 0, 0, 1})
        // .setDstOffset(vk::Offset3D{static_cast<int32_t>(offsetX), static_cast<int32_t>(offsetY), 0})
        .setDstOffset({0, 0, 0})
        .setExtent(vk::Extent3D(renderTargetExtent.width, renderTargetExtent.height, 1));
    mGraphicCommandBuffers[mFrameIndex]->copyImage(
        mLastFrameImages[mFrameIndex]->get()->GetHandle(), vk::ImageLayout::eTransferSrcOptimal,
        mContext->GetSwapchainImages()[mImageIndex], vk::ImageLayout::eTransferDstOptimal, imageCopy);
    // 交换链图像转换为可呈现布局
    vk::ImageMemoryBarrier presentBarrier;
    presentBarrier.setImage(mContext->GetSwapchainImages()[mImageIndex])
        .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
        .setNewLayout(vk::ImageLayout::ePresentSrcKHR)
        .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
        .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
        .setDstAccessMask(vk::AccessFlagBits::eMemoryRead);
    mGraphicCommandBuffers[mFrameIndex]->pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eBottomOfPipe, {}, {}, {}, presentBarrier);
    mGraphicCommandBuffers[mFrameIndex]->end();

    vk::SubmitInfo submitInfo;
    vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    submitInfo.setCommandBuffers(mGraphicCommandBuffers[mFrameIndex].get())
        .setSignalSemaphores(mRenderFinishedSemaphores[mFrameIndex].get())
        .setWaitSemaphores(mImageAvailableSemaphores[mFrameIndex].get())
        .setWaitDstStageMask({waitStage});
    mContext->SubmitToGraphicQueue({submitInfo}, mInFlightFences[mFrameIndex].get());

    vk::PresentInfoKHR presentInfo;
    auto swapchain = mContext->GetSwapchain();
    presentInfo.setSwapchains(swapchain)
        .setImageIndices({mImageIndex})
        .setWaitSemaphores({mRenderFinishedSemaphores[mFrameIndex].get()});
    try
    {
        mContext->SubmitToPresnetQueue(presentInfo);
    }
    catch (vk::OutOfDateKHRError &)
    {
        mLogger->Error("Swapchain out of date, recreating swapchain");
        mContext->GetDevice().waitIdle();
        mContext->RecreateSwapchain();
        auto width = mContext->GetSurfaceInfo().extent.width;
        auto height = mContext->GetSurfaceInfo().extent.height;
        mRenderPassManager->RecreateFrameBuffer(width, height);
        TransitionSwapchainImageLayout();
    }
    mFrameIndex = (mFrameIndex + 1) % mFrameCount;
}
} // namespace MEngine