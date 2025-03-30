#include "System/RenderSystem.hpp"
#include "Logger.hpp"
namespace MEngine
{

RenderSystem::RenderSystem(std::shared_ptr<entt::registry> registry,
                           std::shared_ptr<CommandBufferManager> commandBufferManager,
                           std::shared_ptr<SyncPrimitiveManager> syncPrimitiveManager,
                           std::shared_ptr<RenderPassManager> renderPassManager,
                           std::shared_ptr<PipelineLayoutManager> pipelineLayoutManager,
                           std::shared_ptr<PipelineManager> pipelineManager)
    : mRegistry(registry), mCommandBufferManager(commandBufferManager), mSyncPrimitiveManager(syncPrimitiveManager),
      mRenderPassManager(renderPassManager), mPipelineLayoutManager(pipelineLayoutManager),
      mPipelineManager(pipelineManager)
{
}
void RenderSystem::Init()
{
    auto &context = Context::Instance();
    mFrameCount = context.GetSwapchainImageViews().size();
    mFrameIndex = 0;
    // command buffer
    mGraphicCommandBuffers = mCommandBufferManager->CreatePrimaryCommandBuffers(mFrameCount);
    mSecondaryCommandBuffers = std::vector<std::vector<vk::UniqueCommandBuffer>>(mFrameCount);
    // fence/semaphore
    for (size_t i = 0; i < mFrameCount; ++i)
    {
        mImageAvailableSemaphores.push_back(mSyncPrimitiveManager->CreateUniqueSemaphore());
        mRenderFinishedSemaphores.push_back(mSyncPrimitiveManager->CreateUniqueSemaphore());
        mInFlightFences.push_back(mSyncPrimitiveManager->CreateFence(vk::FenceCreateFlagBits::eSignaled));
    }
    mIsInit = true;
    LogI("RenderSystem Initialized");
}
RenderSystem::~RenderSystem()
{
    if (!mIsShutdown)
        Shutdown();
}
void RenderSystem::Shutdown()
{
    auto &context = Context::Instance();
    context.GetDevice().waitIdle();
    mIsShutdown = true;
    LogI("RenderSystem Shutdown");
}
// Create Done

void RenderSystem::CollectRenderEntities()
{
    auto entities = mRegistry->view<MaterialComponent, MeshComponent>();
    for (auto entity : entities)
    {
        auto &material = mRegistry->get<MaterialComponent>(entity);
        auto &mesh = entities.get<MeshComponent>(entity);
        mBatchMaterialComponents[material.material->GetPipelineType()].push_back(entity);
    }
}

void RenderSystem::Tick(float deltaTime)
{
    CollectRenderEntities();  // Collect same material render entities
    Prepare();                // Prepare
    RenderDefferPass();       // Deffer pass
    RenderShadowDepthPass();  // Shadow pass
    RenderTranslucencyPass(); // Translucency pass
    RenderPostProcessPass();  // Post process pass
    RenderSkyPass();          // Sky pass
    RenderUIPass();           // UI pass
    Present();                // Present
}

void RenderSystem::Prepare()
{
    auto &context = Context::Instance();
    auto result = context.GetDevice().waitForFences({mInFlightFences[mFrameIndex].get()}, VK_TRUE,
                                                    1000000000); // 1s
    if (result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to wait fence");
    }

    context.GetDevice().resetFences(mInFlightFences[mFrameIndex].get());

    mGraphicCommandBuffers[mFrameIndex]->reset();

    auto resultValue = context.GetDevice().acquireNextImageKHR(context.GetSwapchain(), 1000000000,
                                                               mImageAvailableSemaphores[mFrameIndex].get(), nullptr);
    if (resultValue.result == vk::Result::eErrorOutOfDateKHR)
    {
        auto &context = Context::Instance();
        context.GetDevice().waitIdle();
        // TODO:Recreate
        // ReCreate();
    }
    else if (resultValue.result != vk::Result::eSuccess && resultValue.result != vk::Result::eSuboptimalKHR)
    {
        throw std::runtime_error("Failed to acquire next image");
    }
    mImageIndex = resultValue.value;
    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    mGraphicCommandBuffers[mFrameIndex]->begin(beginInfo);
}
void RenderSystem::RenderShadowDepthPass()
{
}
void RenderSystem::RenderDefferPass()
{
    auto &context = Context::Instance();
    vk::RenderPassBeginInfo renderPassBeginInfo;
    std::array<vk::ClearValue, 7> clearValues;
    clearValues[0].color = vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}); // 附件0: 位置
    clearValues[1].color = vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}); // 附件1: 法线
    clearValues[2].color = vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}); // 附件2: Albedo
    clearValues[3].color = vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}); // 附件3: 金属/粗糙度
    clearValues[4].color = vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}); // 附件4: AO
    clearValues[5].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);                        // 附件5: 深度
    clearValues[6].color = vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}); // 附件6: Swapchain
    renderPassBeginInfo.setRenderPass(mRenderPassManager->GetRenderPass(RenderPassType::Deffer))
        .setFramebuffer(mRenderPassManager->GetFrameBuffer(RenderPassType::Deffer, mImageIndex))
        .setRenderArea(vk::Rect2D({0, 0}, context.GetSurfaceInfo().extent))
        .setClearValues(clearValues);
    mGraphicCommandBuffers[mFrameIndex]->beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

    mGraphicCommandBuffers[mFrameIndex]->nextSubpass(vk::SubpassContents::eInline);
    mGraphicCommandBuffers[mFrameIndex]->endRenderPass();
}
void RenderSystem::RenderTranslucencyPass()
{
}
void RenderSystem::RenderPostProcessPass()
{
}
void RenderSystem::RenderSkyPass()
{
}
void RenderSystem::RenderUIPass()
{
}
void RenderSystem::Present()
{
    auto &context = Context::Instance();
    mGraphicCommandBuffers[mFrameIndex]->end();

    vk::SubmitInfo submitInfo;
    vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    submitInfo.setCommandBuffers(mGraphicCommandBuffers[mFrameIndex].get())
        .setSignalSemaphores(mRenderFinishedSemaphores[mFrameIndex].get())
        .setWaitSemaphores(mImageAvailableSemaphores[mFrameIndex].get())
        .setWaitDstStageMask({waitStage});
    context.SubmitToGraphicQueue({submitInfo}, mInFlightFences[mFrameIndex]);

    vk::PresentInfoKHR presentInfo;
    auto swapchain = context.GetSwapchain();
    presentInfo.setSwapchains(swapchain)
        .setImageIndices({mImageIndex})
        .setWaitSemaphores({mRenderFinishedSemaphores[mFrameIndex].get()});
    try
    {
        context.SubmitToPresnetQueue(presentInfo);
    }
    catch (vk::OutOfDateKHRError &)
    {
        context.GetDevice().waitIdle();
        // ReCreate();
    }
    mFrameIndex = (mFrameIndex + 1) % mFrameCount;
}
} // namespace MEngine