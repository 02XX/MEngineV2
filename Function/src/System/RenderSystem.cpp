#include "System/RenderSystem.hpp"
#include "Vertex.hpp"

namespace MEngine
{

RenderSystem::RenderSystem(std::shared_ptr<entt::registry> registry) : mRegistry(registry)
{
    auto &context = Context::Instance();
    // manager
    mSwapchainManager = std::make_unique<SwapchainManager>(Context::Instance().GetSurfaceInfo().extent,
                                                           Context::Instance().GetSurface(), nullptr);
    mCommandBufferManager =
        std::make_unique<CommandBufferManager>(Context::Instance().GetQueueFamilyIndicates().graphicsFamily.value());
    mSyncPrimitiveManager = std::make_unique<SyncPrimitiveManager>();
    mImageManager = std::make_unique<ImageManager>();
    mRenderPassManager = std::make_unique<RenderPassManager>();
    mShaderManager = std::make_unique<ShaderManager>();
    mPipelineLayoutManager = std::make_unique<PipelineLayoutManager>();
    mPipelineManager = std::make_unique<PipelineManager>();
    mFrameCount = mSwapchainManager->GetSwapchainImageViews().size();
    mFrameIndex = 0;
    // command buffer
    mGraphicCommandBuffers = mCommandBufferManager->CreatePrimaryCommandBuffers(mFrameCount);
    mSecondaryCommandBuffers.reserve(mFrameCount);
    // semaphore
    for (size_t i = 0; i < mFrameCount; ++i)
    {
        mImageAvailableSemaphores.push_back(mSyncPrimitiveManager->CreateUniqueSemaphore());
        mRenderFinishedSemaphores.push_back(mSyncPrimitiveManager->CreateUniqueSemaphore());
        mInFlightFences.push_back(mSyncPrimitiveManager->CreateFence(vk::FenceCreateFlagBits::eSignaled));
    }
    // render pass
    mRenderPass = mRenderPassManager->CreateSimpleRenderPass(
        context.GetSurfaceInfo().format.format, vk::ImageLayout::ePresentSrcKHR, vk::Format::eD32SfloatS8Uint);
    // depth stencil
    for (size_t i = 0; i < mFrameCount; ++i)
    {
        mDepthStencilImages.push_back(mImageManager->CreateUniqueDepthStencil(context.GetSurfaceInfo().extent));
        auto depthStencilImageView = mImageManager->CreateImageView(
            mDepthStencilImages[i]->GetImage(), vk::Format::eD32SfloatS8Uint, vk::ComponentMapping{},
            vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, 1, 0, 1});
        mDepthStencilImageViews.push_back(std::move(depthStencilImageView));
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
        mFrameBuffers.push_back(context.GetDevice().createFramebufferUnique(framebufferCreateInfo));
    }
    mShaderManager->LoadShaderModule("forward.vert", "shaders/forward.vert.spv");
    mShaderManager->LoadShaderModule("forward.frag", "shaders/forward.frag.spv");
    InitialPipeline();
}
// Pipeline
void RenderSystem::InitialPipeline()
{
    CreateForwardOpaquePipeline();
    CreateDeferredGBufferPipeline();
    CreateShadowDepthPipeline();
    CreatePostProcessPipeline();
}
void RenderSystem::CreateForwardOpaquePipeline()
{
    auto &context = Context::Instance();
    // 定义PBR材质所需的描述符绑定
    std::vector<DescriptorBindingInfo> pbrBindings = {
        // Set 0: 场景级Uniform数据（每帧更新）
        {.binding = 0,
         .type = vk::DescriptorType::eUniformBuffer,
         .count = 1,
         .stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment},

        // Set 1: 材质纹理（每个材质实例独立）
        {.binding = 1,
         .type = vk::DescriptorType::eCombinedImageSampler, // Albedo
         .count = 1,
         .stageFlags = vk::ShaderStageFlagBits::eFragment},
        {.binding = 2,
         .type = vk::DescriptorType::eCombinedImageSampler, // 法线贴图
         .count = 1,
         .stageFlags = vk::ShaderStageFlagBits::eFragment},
        {.binding = 3,
         .type = vk::DescriptorType::eCombinedImageSampler, // 金属/粗糙度
         .count = 1,
         .stageFlags = vk::ShaderStageFlagBits::eFragment},
        {.binding = 4,
         .type = vk::DescriptorType::eCombinedImageSampler, // AO贴图
         .count = 1,
         .stageFlags = vk::ShaderStageFlagBits::eFragment}};

    // Push Constant用于传递模型矩阵
    std::vector<vk::PushConstantRange> pushConstants = {vk::PushConstantRange()
                                                            .setStageFlags(vk::ShaderStageFlagBits::eVertex)
                                                            .setOffset(0)
                                                            .setSize(sizeof(glm::mat4))};

    auto pipelineLayout = mPipelineLayoutManager->CreateUniquePipelineLayout(pbrBindings, pushConstants);
    GraphicsPipelineConfig config;
    config.pipelineLayout = pipelineLayout.get();
    config.renderPass = mRenderPass.get();
    config.subPass = 0;
    config.vertexShader = mShaderManager->GetShaderModule("forward.vert");
    config.fragmentShader = mShaderManager->GetShaderModule("forward.frag");
    config.vertexBindings = {Vertex::GetVertexInputBindingDescription()};
    auto vertexInputAttributeDescription = Vertex::GetVertexInputAttributeDescription();
    config.vertexAttributes = std::vector<vk::VertexInputAttributeDescription>(vertexInputAttributeDescription.begin(),
                                                                               vertexInputAttributeDescription.end());
    config.topology = vk::PrimitiveTopology::eTriangleList;
    config.polygonMode = vk::PolygonMode::eFill;
    config.lineWidth = 1.0f;
    config.cullMode = vk::CullModeFlagBits::eBack;
    config.frontFace = vk::FrontFace::eCounterClockwise;
    config.viewport = vk::Viewport(0.0f, 0.0f, static_cast<float>(context.GetSurfaceInfo().extent.width),
                                   static_cast<float>(context.GetSurfaceInfo().extent.height), 0.0f, 1.0f);
    config.scissor = vk::Rect2D({0, 0}, context.GetSurfaceInfo().extent);
    config.rasterizationSamples = vk::SampleCountFlagBits::e1;
    config.minSampleShading = 1.0f;
    config.sampleShadingEnable = VK_FALSE;
    config.alphaToCoverageEnable = VK_FALSE;
    config.alphaToOneEnable = VK_FALSE;
    config.sampleMasks = {0xFFFFFFFF};
    config.colorBlendAttachments = {vk::PipelineColorBlendAttachmentState().setColorWriteMask(
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
        vk::ColorComponentFlagBits::eA)};
    config.logicOpEnable = VK_FALSE;
    config.logicOp = vk::LogicOp::eCopy;
    config.blendConstants = {0.0f, 0.0f, 0.0f, 0.0f};
    config.depthTestEnable = VK_TRUE;
    config.depthWriteEnable = VK_TRUE;
    config.depthCompareOp = vk::CompareOp::eLess;
    config.depthBoundsTestEnable = VK_FALSE;
    config.minDepthBounds = 0.0f;
    config.maxDepthBounds = 1.0f;
    config.stencilTestEnable = VK_FALSE;
    config.frontStencilOp = vk::StencilOpState()
                                .setFailOp(vk::StencilOp::eKeep)
                                .setPassOp(vk::StencilOp::eKeep)
                                .setDepthFailOp(vk::StencilOp::eKeep)
                                .setCompareOp(vk::CompareOp::eAlways);
    config.backStencilOp = config.frontStencilOp;
    auto pipeline = mPipelineManager->CreateUniqueGraphicsPipeline(config);
    mPipelines[PipelineType::ForwardOpaque] = std::move(pipeline);
    mPipelineLayouts[PipelineType::ForwardOpaque] = std::move(pipelineLayout);
}
void RenderSystem::CreateDeferredGBufferPipeline()
{
}
void RenderSystem::CreateShadowDepthPipeline()
{
}
void RenderSystem::CreatePostProcessPipeline()
{
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
    auto result = context.GetDevice().waitForFences({mInFlightFences[mFrameIndex].get()}, VK_TRUE,
                                                    1000000000); // 1s
    if (result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to wait fence");
    }

    context.GetDevice().resetFences(mInFlightFences[mFrameIndex].get());

    mGraphicCommandBuffers[mFrameIndex]->reset();

    auto resultValue = context.GetDevice().acquireNextImageKHR(mSwapchainManager->GetSwapchain(), 1000000000,
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
    for (auto &[type, entities] : mBatchMaterialComponents)
    {
        if (type == PipelineType::ForwardOpaque)
        {
            // 每个material批次一个线程
            auto task = Task::Run([this, entities, &mutex, type]() {
                auto secondary = mCommandBufferManager->CreateSecondaryCommandBuffer();
                vk::CommandBufferBeginInfo beginInfo;
                beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
                secondary->begin(beginInfo);

                auto &material = mRegistry->get<MaterialComponent>(entities[0]);
                auto pipeline = mPipelines[type].get();

                secondary->bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
                for (auto entity : entities)
                {
                    auto &meshComponent = mRegistry->get<MeshComponent>(entity);
                    auto vertexBuffer = meshComponent.mesh->GetVertexBuffer();
                    auto indexBuffer = meshComponent.mesh->GetIndexBuffer();
                    auto indexCount = meshComponent.mesh->GetIndexCount();

                    secondary->bindVertexBuffers(0, vertexBuffer, {0});
                    secondary->bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint32);
                    secondary->drawIndexed(indexCount, 1, 0, 0, 0);
                }
                secondary->end();
                std::lock_guard<std::mutex> lock(mutex);
                mSecondaryCommandBuffers[mFrameIndex].push_back(std::move(secondary));
            });
            tasks.push_back(task);
        }
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
    auto swapchain = mSwapchainManager->GetSwapchain();
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