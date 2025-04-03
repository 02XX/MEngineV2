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
        auto imageAvailableSemaphore = mSyncPrimitiveManager->CreateUniqueSemaphore();
        auto renderFinishedSemaphores = mSyncPrimitiveManager->CreateUniqueSemaphore();
        auto inFlightFence = mSyncPrimitiveManager->CreateFence(vk::FenceCreateFlagBits::eSignaled);
        mImageAvailableSemaphores.push_back(std::move(imageAvailableSemaphore));
        mRenderFinishedSemaphores.push_back(std::move(renderFinishedSemaphores));
        mInFlightFences.push_back(std::move(inFlightFence));
    }
    InitUI();
    mIsInit = true;
    LogI("RenderSystem Initialized");
}
void RenderSystem::InitUI()
{
    auto &context = Context::Instance();
    // UI
    // DescriptorPool
    vk::DescriptorPoolCreateInfo poolInfo;
    vk::DescriptorPoolSize poolSize(vk::DescriptorType::eCombinedImageSampler, 1000);
    poolInfo.setMaxSets(1000).setPoolSizes(poolSize).setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
    mUIDescriptorPool = context.GetDevice().createDescriptorPoolUnique(poolInfo);
    if (!mUIDescriptorPool)
    {
        LogE("Failed to create UI descriptor pool");
    }
    //  Initialize ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    mIO = &ImGui::GetIO();
    mIO->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    ImGui::StyleColorsDark();
    // Initialize ImGui Vulkan Backend
    ImGui_ImplSDL3_InitForVulkan(mWindow);
    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.ApiVersion = context.GetInstanceVersion();
    initInfo.Instance = context.GetInstance();
    initInfo.PhysicalDevice = context.GetPhysicalDevice();
    initInfo.Device = context.GetDevice();
    initInfo.QueueFamily = context.GetQueueFamilyIndicates().graphicsFamily.value();
    initInfo.Queue = context.GetGraphicQueue();
    initInfo.DescriptorPool = mUIDescriptorPool.get();
    initInfo.RenderPass = mRenderPassManager->GetRenderPass(RenderPassType::UI);
    initInfo.MinImageCount = context.GetSurfaceInfo().imageCount;
    initInfo.ImageCount = context.GetSurfaceInfo().imageCount;
    initInfo.MSAASamples = static_cast<VkSampleCountFlagBits>(vk::SampleCountFlagBits::e1);
    // optional
    initInfo.Subpass = 0;
    // initInfo.DescriptorPoolSize = 1000;
    ImGui_ImplVulkan_Init(&initInfo);
    LogD("ImGui Vulkan Backend Initialized");
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

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

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
        mBatchMaterialComponents[material.pipelineType].push_back(entity);
    }
}

void RenderSystem::Tick(float deltaTime)
{
    CollectRenderEntities(); // Collect same material render entities
    Prepare();               // Prepare
    // RenderDefferPass();       // Deffer pass
    // RenderShadowDepthPass();  // Shadow pass
    RenderTranslucencyPass(); // Translucency pass
    // RenderPostProcessPass();  // Post process pass
    // RenderSkyPass();          // Sky pass
    // RenderUIPass(); // UI pass
    Present(); // Present
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

    context.GetDevice().resetFences({mInFlightFences[mFrameIndex].get()});

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
    auto &context = Context::Instance();
    auto extent = context.GetSurfaceInfo().extent;
    // Translucency entities
    auto entities = mBatchMaterialComponents[PipelineType::Translucency];
    auto pipelineLayout = mPipelineLayoutManager->GetPipelineLayout(PipelineLayoutType::TranslucencyLayout);
    auto pipeline = mPipelineManager->GetPipeline(PipelineType::Translucency);
    auto renderPass = mRenderPassManager->GetRenderPass(RenderPassType::Translucency);
    auto frameBuffer = mRenderPassManager->GetFrameBuffer(RenderPassType::Translucency, mImageIndex);
    vk::RenderPassBeginInfo renderPassBeginInfo;
    std::array<vk::ClearValue, 2> clearValues{
        vk::ClearValue(std::array<float, 4>{1.0f, 1.0f, 0.0f, 1.0f}), // 附件0: swapchain
        vk::ClearDepthStencilValue(1.0f, 0)                           // 附件1: depth
    };
    clearValues[0].color = vk::ClearColorValue(std::array<float, 4>{0.0f, 1.0f, 0.0f, 1.0f}); // 附件0: Swapchain
    renderPassBeginInfo.setRenderPass(renderPass)
        .setFramebuffer(frameBuffer)
        .setRenderArea(vk::Rect2D({0, 0}, extent))
        .setClearValues(clearValues);
    mGraphicCommandBuffers[mFrameIndex]->beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
    {
        // // 1. 绑定管线
        // mGraphicCommandBuffers[mFrameIndex]->bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
        // // 2. 绑定描述符集
        // for (auto entity : entities)
        // {
        //     auto &material = mRegistry->get<MaterialComponent>(entity);
        //     auto &mesh = mRegistry->get<MeshComponent>(entity);
        //     // auto descriptorSet = mPipelineLayoutManager->GetDescriptorSet(material.pipelineLayoutType, entity);
        //     // mGraphicCommandBuffers[mFrameIndex]->bindDescriptorSets(
        //     //     vk::PipelineBindPoint::eGraphics,
        //     //     mPipelineLayoutManager->GetPipelineLayout(material.pipelineLayoutType), 0, descriptorSet, {});
        //     // 3. 绑定顶点缓冲区
        //     auto vertexBuffer = mesh.mesh->GetVertexBuffer();
        //     mGraphicCommandBuffers[mFrameIndex]->bindVertexBuffers(0, vertexBuffer, {0});
        //     // 4. 绑定索引缓冲区
        //     auto indexBuffer = mesh.mesh->GetIndexBuffer();
        //     mGraphicCommandBuffers[mFrameIndex]->bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint32);
        //     // 5. 绘制
        //     mGraphicCommandBuffers[mFrameIndex]->drawIndexed(mesh.mesh->GetIndexCount(), 1, 0, 0, 0);
        // }
    }
    mGraphicCommandBuffers[mFrameIndex]->endRenderPass();
}
void RenderSystem::RenderPostProcessPass()
{
}
void RenderSystem::RenderSkyPass()
{
}
void RenderSystem::RenderUIPass()
{
    vk::ClearValue clearValue(std::array<float, 4>{1.0f, 1.0f, 0.0f, 1.0f});
    vk::RenderPassBeginInfo renderPassBeginInfo;
    auto frameBuffer = mRenderPassManager->GetFrameBuffer(RenderPassType::UI, mImageIndex);
    auto renderPass = mRenderPassManager->GetRenderPass(RenderPassType::UI);
    renderPassBeginInfo.setRenderPass(renderPass)
        .setFramebuffer(frameBuffer)
        .setRenderArea(vk::Rect2D({0, 0}, Context::Instance().GetSurfaceInfo().extent))
        .setClearValues(clearValue);
    mGraphicCommandBuffers[mFrameIndex]->beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("Debug Window", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("If you can see this, UI is working!");
    ImGui::End();
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
    context.SubmitToGraphicQueue({submitInfo}, mInFlightFences[mFrameIndex].get());

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