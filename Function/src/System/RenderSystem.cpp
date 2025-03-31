#include "System/RenderSystem.hpp"
namespace MEngine
{

RenderSystem::RenderSystem(SDL_Window *window, std::shared_ptr<entt::registry> registry,
                           std::shared_ptr<CommandBufferManager> commandBufferManager,
                           std::shared_ptr<SyncPrimitiveManager> syncPrimitiveManager,
                           std::shared_ptr<RenderPassManager> renderPassManager,
                           std::shared_ptr<PipelineLayoutManager> pipelineLayoutManager,
                           std::shared_ptr<PipelineManager> pipelineManager)
    : mRegistry(registry), mCommandBufferManager(commandBufferManager), mSyncPrimitiveManager(syncPrimitiveManager),
      mRenderPassManager(renderPassManager), mPipelineLayoutManager(pipelineLayoutManager),
      mPipelineManager(pipelineManager), mWindow(window)
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
    // UI
    //  Initialize ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    mIO = &ImGui::GetIO();
    mIO->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

    // Initialize ImGui Vulkan Backend
    ImGui_ImplSDL3_InitForVulkan(mWindow);
    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.ApiVersion = context.GetInstanceVersion();
    initInfo.Instance = context.GetInstance();
    initInfo.PhysicalDevice = context.GetPhysicalDevice();
    initInfo.Device = context.GetDevice();
    initInfo.QueueFamily = context.GetQueueFamilyIndicates().graphicsFamily.value();
    initInfo.Queue = context.GetGraphicQueue();
    // initInfo.DescriptorPool;
    initInfo.RenderPass = mRenderPassManager->GetRenderPass(RenderPassType::UI);
    initInfo.MinImageCount = context.GetSurfaceInfo().imageCount;
    initInfo.ImageCount = context.GetSurfaceInfo().imageCount;
    initInfo.MSAASamples = static_cast<VkSampleCountFlagBits>(vk::SampleCountFlagBits::e1);
    initInfo.DescriptorPoolSize = 1000;
    // optional
    initInfo.Subpass = 0;
    ImGui_ImplVulkan_Init(&initInfo);
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
    CollectRenderEntities(); // Collect same material render entities
    Prepare();               // Prepare
    // RenderDefferPass();       // Deffer pass
    // RenderShadowDepthPass();  // Shadow pass
    // RenderTranslucencyPass(); // Translucency pass
    // RenderPostProcessPass();  // Post process pass
    // RenderSkyPass();          // Sky pass
    RenderUIPass(); // UI pass
    Present();      // Present
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
    // 转换图像布局
    // vk::ImageMemoryBarrier barrier;
    // barrier.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
    //     .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead)
    //     .setOldLayout(vk::ImageLayout::ePresentSrcKHR)
    //     .setNewLayout(vk::ImageLayout::eColorAttachmentOptimal)
    //     .setImage(mRenderPassManager->GetUIFrameResource(mImageIndex).swapchainImage)
    //     .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
    // mGraphicCommandBuffers[mFrameIndex]->pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput,
    //                                                      vk::PipelineStageFlagBits::eColorAttachmentOutput, {},
    //                                                      nullptr, nullptr, barrier);

    vk::ClearValue clearValue(std::array<float, 4>{0, 0, 0, 0});
    vk::RenderPassBeginInfo renderPassBeginInfo;
    renderPassBeginInfo.setRenderPass(mRenderPassManager->GetRenderPass(RenderPassType::UI))
        .setFramebuffer(mRenderPassManager->GetFrameBuffer(RenderPassType::UI, mImageIndex))
        .setRenderArea(vk::Rect2D({0, 0}, Context::Instance().GetSurfaceInfo().extent))
        .setClearValues(clearValue);
    mGraphicCommandBuffers[mFrameIndex]->beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    ImGui::ShowDemoWindow();
    ImGui::Render();
    ImDrawData *drawData = ImGui::GetDrawData();
    bool isMinimized = (drawData->DisplaySize.x <= 0.0f || drawData->DisplaySize.y <= 0.0f);
    if (!isMinimized)
    {
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), mGraphicCommandBuffers[mFrameIndex].get());
    }
    mGraphicCommandBuffers[mFrameIndex]->endRenderPass();
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