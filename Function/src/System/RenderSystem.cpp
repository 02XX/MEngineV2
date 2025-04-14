#include "System/RenderSystem.hpp"
#include <vulkan/vulkan.hpp>

namespace MEngine
{

RenderSystem::RenderSystem(
    std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context, std::shared_ptr<entt::registry> registry,
    std::shared_ptr<RenderPassManager> renderPassManager, std::shared_ptr<PipelineLayoutManager> pipelineLayoutManager,
    std::shared_ptr<PipelineManager> pipelineManager, std::shared_ptr<CommandBufferManager> commandBufferManager,
    std::shared_ptr<SyncPrimitiveManager> syncPrimitiveManager, std::shared_ptr<DescriptorManager> descriptorManager,
    std::shared_ptr<SamplerManager> samplerManager, std::shared_ptr<BufferFactory> bufferFactory,
    std::shared_ptr<ImageFactory> imageFactory, std::shared_ptr<IWindow> window, std::shared_ptr<UI> ui)
    : mLogger(logger), mContext(context), mRegistry(registry), mRenderPassManager(renderPassManager),
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
    mWindow->SetEventCallback([this](const void *event) { mUI->ProcessEvent(static_cast<const SDL_Event *>(event)); });
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
// Create Done

void RenderSystem::CollectRenderEntities()
{
    auto needDeleteEntities = mUI->GetNeedDeleteEntities();
    for (auto entity : needDeleteEntities)
    {
        mRegistry->destroy(entity);
    }
    mUI->ClearNeedDeleteEntities();
    mBatchMaterialComponents.clear();
    auto entities = mRegistry->view<MaterialComponent, MeshComponent>();
    for (auto entity : entities)
    {
        auto &material = mRegistry->get<MaterialComponent>(entity);
        auto &mesh = entities.get<MeshComponent>(entity);
        mBatchMaterialComponents[material.material->GetPipelineType()].push_back(entity);
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
    CollectRenderEntities(); // Collect same material render entities
    // TickRotationMatrix();
    CollectMainCamera();
    Prepare(); // Prepare
    // RenderShadowDepthPass();  // Shadow pass
    // RenderMainPass();       // Deffer pass
    // RenderSkyPass();          // Sky pass
    RenderTranslucencyPass(); // Translucency pass
    // RenderPostProcessPass();  // Post process pass
    RenderUIPass(deltaTime); // UI pass
    Present();               // Present
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

        mContext->GetDevice().waitIdle();
        mContext->RecreateSwapchain();
        mRenderPassManager->RecreateUIFrameBuffer();
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

    mUI->RenderUI();

    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    mGraphicCommandBuffers[mFrameIndex]->begin(beginInfo);
}
void RenderSystem::RenderShadowDepthPass()
{
}
void RenderSystem::RenderMainPass()
{
    auto extent = mRenderPassManager->GetExtent();
    vk::RenderPassBeginInfo renderPassBeginInfo;
    std::array<vk::ClearValue, 7> clearValues;
    clearValues[0].color = vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}); // 附件0: 位置
    clearValues[1].color = vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}); // 附件1: 法线
    clearValues[2].color = vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}); // 附件2: Albedo
    clearValues[3].color = vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}); // 附件3: 金属/粗糙度
    clearValues[4].color = vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}); // 附件4: AO
    clearValues[5].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);                        // 附件5: 深度
    clearValues[6].color = vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}); // 附件6: Swapchain
    auto defferFrameBuffers = mRenderPassManager->GetFrameBuffer(RenderPassType::MainPass);
    renderPassBeginInfo.setRenderPass(mRenderPassManager->GetRenderPass(RenderPassType::MainPass))
        .setFramebuffer(defferFrameBuffers[mImageIndex])
        .setRenderArea(vk::Rect2D({0, 0}, extent))
        .setClearValues(clearValues);
    mGraphicCommandBuffers[mFrameIndex]->beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

    mGraphicCommandBuffers[mFrameIndex]->nextSubpass(vk::SubpassContents::eInline);
    mGraphicCommandBuffers[mFrameIndex]->endRenderPass();
}
void RenderSystem::RenderTranslucencyPass()
{
    auto extent = mRenderPassManager->GetExtent();
    // Translucency entities
    auto entities = mBatchMaterialComponents[PipelineType::ForwardTransparent];
    auto pipelineLayout = mPipelineLayoutManager->GetPipelineLayout(PipelineLayoutType::Transparent);
    auto pipeline = mPipelineManager->GetPipeline(PipelineType::ForwardTransparent);
    auto renderPass = mRenderPassManager->GetRenderPass(RenderPassType::Transparent);
    auto transparentFrameBuffers = mRenderPassManager->GetFrameBuffer(RenderPassType::Transparent);
    vk::RenderPassBeginInfo renderPassBeginInfo;
    std::array<vk::ClearValue, 2> clearValues{
        vk::ClearValue(std::array<float, 4>{0.1f, 0.1f, 0.1f, 1.0f}), // 附件0: Render Target
        vk::ClearDepthStencilValue(1.0f, 0)                           // 附件1: Depth Stencil
    };
    renderPassBeginInfo.setRenderPass(renderPass)
        .setFramebuffer(transparentFrameBuffers[mImageIndex])
        .setRenderArea(vk::Rect2D({0, 0}, extent))
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
        scissor.setOffset({0, 0}).setExtent(extent);
        mGraphicCommandBuffers[mFrameIndex]->setScissor(0, scissor);
        // 1. 绑定管线
        mGraphicCommandBuffers[mFrameIndex]->bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

        for (auto entity : entities)
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
            auto materialDescriptorSet = material.material->GetMaterialDescriptorSet();
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
    mUI->SetImageIndex(mImageIndex);
    auto queueFamilyIndices = mContext->GetQueueFamilyIndicates();
    vk::ClearValue clearValue(std::array<float, 4>{0.1f, 0.1f, 0.1f, 1.0f});
    vk::RenderPassBeginInfo renderPassBeginInfo;
    auto uiframeBuffers = mRenderPassManager->GetFrameBuffer(RenderPassType::UI);
    auto renderPass = mRenderPassManager->GetRenderPass(RenderPassType::UI);
    auto &uiFrameResources = mRenderPassManager->GetUIFrameResource();

    // 渲染前的布局转换
    vk::ImageMemoryBarrier preRenderBarrier;
    preRenderBarrier.setImage(uiFrameResources[mImageIndex]->renderTargetImage)
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
        .setRenderArea(vk::Rect2D({0, 0}, mContext->GetSurfaceInfo().extent))
        .setClearValues(clearValue);

    mGraphicCommandBuffers[mFrameIndex]->beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
    mUI->RecordUICommandBuffer(mGraphicCommandBuffers[mFrameIndex].get());
    mGraphicCommandBuffers[mFrameIndex]->endRenderPass();
}
void RenderSystem::Present()
{

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
        mContext->GetDevice().waitIdle();
        mContext->RecreateSwapchain();
        mRenderPassManager->RecreateUIFrameBuffer();
    }
    mFrameIndex = (mFrameIndex + 1) % mFrameCount;
}
} // namespace MEngine