#include "System/RenderSystem.hpp"
#include <vector>

namespace MEngine
{

RenderSystem::RenderSystem(std::shared_ptr<entt::registry> registry) : mRegistry(registry)
{
    auto &context = Context::Instance();
    // manager
    mSwapchainManager = std::make_unique<SwapchainManager>(Context::Instance().GetSurfaceInfo().extent,
                                                           Context::Instance().GetSurface().get(), nullptr);
    mCommandBufferManager =
        std::make_unique<CommandBufferManager>(Context::Instance().GetQueueFamilyIndicates().graphicsFamily.value());
    mSyncPrimitiveManager = std::make_unique<SyncPrimitiveManager>();
    mImageManager = std::make_unique<ImageManager>();
    mTaskScheduler = std::make_unique<TaskScheduler>(4, 1000);
    mRenderPassManager = std::make_unique<RenderPassManager>();
    mFrameCount = mSwapchainManager->GetSwapchainImageViews().size();
    mFrameIndex = 0;

    // command buffer
    mGraphicCommandBuffers = mCommandBufferManager->CreatePrimaryCommandBuffers(mFrameCount);
    mSecondaryCommandBuffers.reserve(mFrameCount);
    // semaphore
    mImageAvailableSemaphores.resize(mFrameCount);
    for (size_t i = 0; i < mFrameCount; ++i)
    {
        mImageAvailableSemaphores[i] = mSyncPrimitiveManager->CreateUniqueSemaphore();
        mRenderFinishedSemaphores[i] = mSyncPrimitiveManager->CreateUniqueSemaphore();
        mInFlightFences.push_back(mSyncPrimitiveManager->CreateFence(vk::FenceCreateFlagBits::eSignaled));
    }
    // render pass
    mRenderPass = mRenderPassManager->CreateSimpleRenderPass(
        context.GetSurfaceInfo().format.format, vk::ImageLayout::ePresentSrcKHR, vk::Format::eD32SfloatS8Uint);
    // depth stencil
    for (size_t i = 0; i < mFrameCount; ++i)
    {
        mDepthStencilImages.push_back(mImageManager->CreateUniqueDepthStencil(context.GetSurfaceInfo().extent));
        mDepthStencilImageViews.push_back(
            mImageManager->CreateImageView(mDepthStencilImages[i]->GetImage(), vk::Format::eD32SfloatS8Uint));
    }
    //  frame buffer
    auto swapchainImageViews = mSwapchainManager->GetSwapchainImageViews();
    mFrameBuffers.reserve(mFrameCount);
    for (size_t i = 0; i < mFrameCount; ++i)
    {
        std::array<vk::ImageView, 2> attachments = {swapchainImageViews[i], mDepthStencilImageViews[i].get()};
        vk::FramebufferCreateInfo framebufferCreateInfo;
        framebufferCreateInfo.setRenderPass(mRenderPass.get())
            .setAttachments(attachments)
            .setWidth(context.GetSurfaceInfo().extent.width)
            .setHeight(context.GetSurfaceInfo().extent.height)
            .setLayers(1);
        mFrameBuffers.push_back(context.GetDevice()->createFramebufferUnique(framebufferCreateInfo));
    }
}

void RenderSystem::CollectRenderEntities()
{
    auto entities = mRegistry->view<MaterialComponent, MeshComponent>();
    for (auto entity : entities)
    {
        auto &material = mRegistry->get<MaterialComponent>(entity);
        auto &mesh = entities.get<MeshComponent>(entity);
        mBatchMaterialComponents[mesh.GetID()].push_back(entity);
    }
}

void RenderSystem::Tick()
{
    CollectRenderEntities(); // Collect same material render entities
    Prepare();
    RenderForwardPass();
    Present();
}
void RenderSystem::Prepare()
{
    auto &context = Context::Instance();
    auto result = context.GetDevice()->waitForFences({mInFlightFences[mFrameIndex].get()}, VK_TRUE,
                                                     1000000000); // 1s
    if (result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to wait fence");
    }

    context.GetDevice()->resetFences(mInFlightFences[mFrameIndex].get());

    mGraphicCommandBuffers[mFrameIndex]->reset();

    auto resultValue = context.GetDevice()->acquireNextImageKHR(mSwapchain.get(), 1000000000,
                                                                mImageAvailableSemaphores[mFrameIndex].get(), nullptr);
    if (resultValue.result == vk::Result::eErrorOutOfDateKHR)
    {
        auto &context = Context::Instance();
        context.GetDevice()->waitIdle();
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

    vk::RenderPassBeginInfo renderPassBeginInfo;
    std::vector<vk::ClearValue> clearValues = {vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}),
                                               vk::ClearDepthStencilValue(1.0f, 0)};

    renderPassBeginInfo.setRenderPass(mRenderPass.get())
        .setRenderArea(vk::Rect2D({0, 0}, Context::Instance().GetSurfaceInfo().extent))
        .setFramebuffer(mFrameBuffers[mFrameIndex].get())
        .setClearValues(clearValues);
    mGraphicCommandBuffers[mFrameIndex]->beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
}
void RenderSystem::RenderForwardPass()
{
    auto &context = Context::Instance();
    std::mutex mutex;
    mSecondaryCommandBuffers.clear();
    std::vector<std::shared_ptr<Task>> tasks;
    for (auto &[materialID, entities] : mBatchMaterialComponents)
    {
        if (materialID == 9)
        {
            continue; // deffered pass
        }
        // 每个material批次一个线程
        auto task = Task::Run([this, entities, &mutex]() {
            auto secondary = mCommandBufferManager->CreateSecondaryCommandBuffer();
            vk::CommandBufferBeginInfo beginInfo;
            beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
            secondary->begin(beginInfo);

            auto &material = mRegistry->get<MaterialComponent>(entities[0]);
            auto pipeline = material.GetPipeline();
            secondary->bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.get());
            for (auto entity : entities)
            {
                auto &mesh = mRegistry->get<MeshComponent>(entity);
                auto &vertexBuffer = mesh.GetVertexBuffer();
                auto &indexBuffer = mesh.GetIndexBuffer();

                secondary->bindVertexBuffers(0, vertexBuffer->GetBuffer(), {0});
                secondary->bindIndexBuffer(indexBuffer->GetBuffer(), 0, mesh.GetIndexType());
                secondary->drawIndexed(mesh.GetIndexCount(), 1, mesh.GetFirstIndex(), mesh.GetVertexOffset(), 0);
            }
            secondary->end();
            std::lock_guard<std::mutex> lock(mutex);
            mSecondaryCommandBuffers[mFrameIndex].push_back(std::move(secondary));
        });
        tasks.push_back(task);
    }
    Task::WhenAll(tasks);
    std::vector<vk::CommandBuffer> rawCommandBuffers;
    rawCommandBuffers.reserve(mSecondaryCommandBuffers.size());
    std::transform(mSecondaryCommandBuffers[mFrameIndex].begin(), mSecondaryCommandBuffers[mFrameIndex].end(),
                   rawCommandBuffers.begin(), [](auto &secondary) { return secondary.get(); });
    mGraphicCommandBuffers[mFrameIndex]->executeCommands(rawCommandBuffers);
}
void RenderSystem::Present()
{
    auto &context = Context::Instance();
    mGraphicCommandBuffers[mFrameIndex]->endRenderPass();
    mGraphicCommandBuffers[mFrameIndex]->end();

    vk::SubmitInfo submitInfo;
    vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    submitInfo.setCommandBuffers(mGraphicCommandBuffers[mFrameIndex].get())
        .setSignalSemaphores(mRenderFinishedSemaphores[mFrameIndex].get())
        .setWaitSemaphores(mImageAvailableSemaphores[mFrameIndex].get())
        .setWaitDstStageMask({waitStage});
    context.SubmitToGraphicQueue({submitInfo}, mInFlightFences[mFrameIndex]);

    vk::PresentInfoKHR presentInfo;
    presentInfo.setSwapchains({mSwapchain.get()})
        .setImageIndices({mImageIndex})
        .setWaitSemaphores({mRenderFinishedSemaphores[mFrameIndex].get()});
    try
    {
        context.SubmitToPresnetQueue(presentInfo);
    }
    catch (vk::OutOfDateKHRError &)
    {
        context.GetDevice()->waitIdle();
        // ReCreate();
    }
    mFrameIndex = (mFrameIndex + 1) % mFrameCount;
}
} // namespace MEngine