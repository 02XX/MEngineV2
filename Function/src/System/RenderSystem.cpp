#include "System/RenderSystem.hpp"
#include "VMA.hpp"
#include "stb_image.h"
#include <cstring>
namespace MEngine
{

RenderSystem::RenderSystem(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                           std::shared_ptr<IConfigure> configure, std::shared_ptr<entt::registry> registry,
                           std::shared_ptr<RenderPassManager> renderPassManager,
                           std::shared_ptr<PipelineLayoutManager> pipelineLayoutManager,
                           std::shared_ptr<PipelineManager> pipelineManager,
                           std::shared_ptr<CommandBufferManager> commandBufferManager,
                           std::shared_ptr<SyncPrimitiveManager> syncPrimitiveManager,
                           std::shared_ptr<DescriptorManager> descriptorManager,
                           std::shared_ptr<BufferFactory> bufferFactory, std::shared_ptr<ImageFactory> imageFactory,
                           std::shared_ptr<SamplerManager> samplerManager)
    : System(logger, context, configure, registry), mRenderPassManager(renderPassManager),
      mPipelineLayoutManager(pipelineLayoutManager), mPipelineManager(pipelineManager),
      mCommandBufferManager(commandBufferManager), mSyncPrimitiveManager(syncPrimitiveManager),
      mDescriptorManager(descriptorManager), mBufferFactory(bufferFactory), mImageFactory(imageFactory),
      mSamplerManager(samplerManager)
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
    // Uniform Buffer
    mCameraUBO = mBufferFactory->CreateBuffer(BufferType::Uniform, sizeof(CameraUniform));
    auto globalDescriptorSetLayout = mPipelineLayoutManager->GetGlobalDescriptorSetLayout();
    for (uint32_t i = 0; i < mFrameCount; ++i)
    {
        auto sets = mDescriptorManager->AllocateUniqueDescriptorSet({globalDescriptorSetLayout});
        mGlobalDescriptorSets.push_back(std::move(sets[0]));
    }
    mCameraUBO = mBufferFactory->CreateBuffer(BufferType::Uniform, sizeof(CameraUniform));
    for (size_t i = 0; i < mLightSBOs.size(); ++i)
    {
        mLightSBOs[i] = mBufferFactory->CreateBuffer(BufferType::Uniform, sizeof(LightUniform));
    }
    EnvMap();
    InitialRenderTargetImageLayout();
    InitialSwapchainImageLayout();
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
void RenderSystem::InitialRenderTargetImageLayout()
{
    auto fence = mSyncPrimitiveManager->CreateFence();
    std::vector<vk::SubmitInfo> submitInfos;
    auto renderTargetCommandBuffer = mCommandBufferManager->CreatePrimaryCommandBuffer(CommandBufferType::Graphic);
    renderTargetCommandBuffer->reset();
    // RenderTarget ImageLayout
    renderTargetCommandBuffer->begin(vk::CommandBufferBeginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
    {
        auto renderTargetImages = mRenderPassManager->GetRenderTargets();
        for (auto &renderTarget : renderTargetImages)
        {
            // Render Target ImageLayout
            vk::ImageMemoryBarrier forwardImageMemoryBarrier;
            forwardImageMemoryBarrier.setImage(renderTarget.colorImage->GetHandle())
                .setOldLayout(vk::ImageLayout::eUndefined)
                .setNewLayout(vk::ImageLayout::eColorAttachmentOptimal)
                .setSrcAccessMask(vk::AccessFlagBits::eNone)
                .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
                .setSubresourceRange(vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
            renderTargetCommandBuffer->pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe,
                                                       vk::PipelineStageFlagBits::eColorAttachmentOutput, {}, {}, {},
                                                       forwardImageMemoryBarrier);
            // Depth Stencil ImageLayout
            vk::ImageMemoryBarrier depthImageMemoryBarrier;
            depthImageMemoryBarrier.setImage(renderTarget.depthStencilImage->GetHandle())
                .setOldLayout(vk::ImageLayout::eUndefined)
                .setNewLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
                .setSrcAccessMask(vk::AccessFlagBits::eNone)
                .setDstAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite |
                                  vk::AccessFlagBits::eDepthStencilAttachmentRead)
                .setSubresourceRange(vk::ImageSubresourceRange{
                    vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, 1, 0, 1});
            renderTargetCommandBuffer->pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe,
                                                       vk::PipelineStageFlagBits::eEarlyFragmentTests, {}, {}, {},
                                                       depthImageMemoryBarrier);
        }
    }
    renderTargetCommandBuffer->end();
    vk::SubmitInfo submitInfo;
    submitInfo.setCommandBuffers({renderTargetCommandBuffer.get()});
    submitInfos.push_back(submitInfo);
    // 提交命令缓冲区
    mContext->SubmitToGraphicQueue(submitInfos, fence.get());
    auto result = mContext->GetDevice().waitForFences({fence.get()}, VK_TRUE, 1000000000); // 1s
    if (result != vk::Result::eSuccess)
    {
        mLogger->Error("Failed to transition render target image layout");
        throw std::runtime_error("Failed to transition render target image layout");
    }
    mLogger->Info("RenderTarget imageLayout transitioned successfully");
}
void RenderSystem::InitialSwapchainImageLayout()
{
    auto fence = mSyncPrimitiveManager->CreateFence();
    std::vector<vk::SubmitInfo> submitInfos;
    auto swapchainCommandBuffer = mCommandBufferManager->CreatePrimaryCommandBuffer(CommandBufferType::Graphic);
    swapchainCommandBuffer->reset();
    // SwapchainImageLayout
    swapchainCommandBuffer->begin(vk::CommandBufferBeginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
    {

        auto swapchainImages = mContext->GetSwapchainImages();
        for (auto swapchainImage : swapchainImages)
        {
            vk::ImageMemoryBarrier swapchainImageMemoryBarrier;
            swapchainImageMemoryBarrier.setImage(swapchainImage)
                .setOldLayout(vk::ImageLayout::eUndefined)
                .setNewLayout(vk::ImageLayout::ePresentSrcKHR)
                .setSrcAccessMask(vk::AccessFlagBits::eNone)
                .setDstAccessMask(vk::AccessFlagBits::eNone)
                .setSubresourceRange(vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
            swapchainCommandBuffer->pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe,
                                                    vk::PipelineStageFlagBits::eTopOfPipe, {}, {}, {},
                                                    swapchainImageMemoryBarrier);
        }
    }
    swapchainCommandBuffer->end();
    vk::SubmitInfo submitInfo;
    submitInfo.setCommandBuffers({swapchainCommandBuffer.get()});
    submitInfos.push_back(submitInfo);
    // 提交命令缓冲区
    mContext->SubmitToGraphicQueue(submitInfos, fence.get());
    auto result = mContext->GetDevice().waitForFences({fence.get()}, VK_TRUE, 1000000000); // 1s
    if (result != vk::Result::eSuccess)
    {
        mLogger->Error("Failed to transition swapchain image layout");
        throw std::runtime_error("Failed to transition swapchain image layout");
    }
    mLogger->Info("Swapchain imageLayout transitioned successfully");
}
void RenderSystem::EnvMap()
{
    // float *rgba = nullptr;
    int width = 0, height = 0;
    const char *err = nullptr;
    auto exrPath = std::filesystem::current_path() / "Resource/Texture/meadow_4k.exr";
    auto defaultTexturePath = std::filesystem::current_path() / "Resource/Material/bg.png";
    auto rgba = stbi_load(defaultTexturePath.string().c_str(), &width, &height, nullptr, 4);
    // LoadEXR(&rgba, &width, &height, exrPath.string().c_str(), &err);
    if (err != nullptr)
    {
        mLogger->Error("Failed to load EXR file: {}", err);
        throw std::runtime_error("Failed to load EXR file");
    }
    mLogger->Info("EXR file loaded successfully: {}x{}", width, height);
    // 创建纹理图像
    vk::ImageCreateInfo imageCreateInfo;
    imageCreateInfo.setImageType(vk::ImageType::e2D)
        .setFormat(vk::Format::eR32G32B32A32Sfloat)
        .setExtent(vk::Extent3D(width, height, 1))
        .setMipLevels(1)
        .setArrayLayers(1)
        .setSamples(vk::SampleCountFlagBits::e1)
        .setTiling(vk::ImageTiling::eOptimal)
        .setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst)
        .setSharingMode(vk::SharingMode::eExclusive)
        .setInitialLayout(vk::ImageLayout::eUndefined);
    mEnvMap =
        mImageFactory->CreateImage(ImageType::Texture2D, vk::Extent3D(width, height, 1), width * height * 4, rgba);
    mEnvMapImageView = mImageFactory->CreateImageView(mEnvMap.get());
    mEnvMapSampler =
        mSamplerManager->CreateUniqueSampler(vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear);
    // 上传DescriptorSet
    // mDescriptorManager->UpdateCombinedSamplerImageDescriptorSet(
    //     std::vector<ImageDescriptor>{{mEnvMapImageView.get(), mEnvMapSampler.get()}}, 2,
    //     mGlobalDescriptorSets[mFrameIndex].get());

    for (size_t i = 0; i < mFrameCount; ++i)
    {
        vk::WriteDescriptorSet writeDescriptorSet;
        vk::DescriptorImageInfo imageInfo;
        imageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
            .setImageView(mEnvMapImageView.get())
            .setSampler(mEnvMapSampler.get());
        writeDescriptorSet.setDstSet(mGlobalDescriptorSets[i].get())
            .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
            .setImageInfo(imageInfo)
            .setDstBinding(2)
            .setDstArrayElement(0);
        mContext->GetDevice().updateDescriptorSets({writeDescriptorSet}, {});
    }
}
void RenderSystem::CollectEntities()
{
    mRenderEntities.clear();
    auto renderEntities = mRegistry->view<MaterialComponent, MeshComponent>();
    for (auto entity : renderEntities)
    {
        auto &material = mRegistry->get<MaterialComponent>(entity);
        auto &mesh = mRegistry->get<MeshComponent>(entity);
        mRenderEntities[material.material->GetRenderType()].push_back(entity);
    }
    auto entities = mRegistry->view<CameraComponent>();
    for (auto entity : entities)
    {
        auto &camera = entities.get<CameraComponent>(entity);
        auto &transform = mRegistry->get<TransformComponent>(entity);
        if (camera.isMainCamera)
        {
            mMainCameraEntity = entity;
            CameraUniform cameraUniform;
            cameraUniform.position = transform.position;
            cameraUniform.view = camera.viewMatrix;
            cameraUniform.projection = camera.projectionMatrix;
            // 设置Uniform Buffer
            memcpy(mCameraUBO->GetAllocationInfo().pMappedData, &cameraUniform, sizeof(CameraUniform));
            break;
        }
    }
    std::vector<std::reference_wrapper<Buffer>> cameraUBORefs = {*mCameraUBO};
    mDescriptorManager->UpdateUniformDescriptorSet(cameraUBORefs, 0, mGlobalDescriptorSets[mFrameIndex].get());

    auto lightEntities = mRegistry->view<LightComponent, TransformComponent>();
    int lightIndex = 0;
    for (auto entity : lightEntities)
    {
        if (lightIndex >= mLightSBOs.size())
            break; // 超过最大光源数量
        auto &lightComponent = mRegistry->get<LightComponent>(entity);
        auto &transformComponent = mRegistry->get<TransformComponent>(entity);
        auto &lightSBO = mLightSBOs[lightIndex];
        LightUniform lightUniform;
        lightUniform.position = transformComponent.position;
        lightUniform.range = lightComponent.range;
        lightUniform.direction = transformComponent.rotation * glm::vec3(0.0f, 1.0f, 0.0f);
        lightUniform.coneAngle = lightComponent.coneAngle;
        lightUniform.color = lightComponent.color;
        lightUniform.intensity = lightComponent.intensity;
        lightUniform.type = lightComponent.type;
        // 更新光源数据
        memcpy(lightSBO->GetAllocationInfo().pMappedData, &lightUniform, sizeof(LightUniform));
        lightIndex++;
    }
    // 更新光源描述符集
    std::vector<Buffer *> lightSBOsPointers;
    lightSBOsPointers.reserve(mLightSBOs.size());
    for (const auto &lightSBO : mLightSBOs)
    {
        lightSBOsPointers.push_back(lightSBO.get());
    }
    std::vector<std::reference_wrapper<Buffer>> lightSBOsReferences;
    lightSBOsReferences.reserve(lightSBOsPointers.size());
    for (auto *lightSBO : lightSBOsPointers)
    {
        lightSBOsReferences.emplace_back(*lightSBO);
    }
    mDescriptorManager->UpdateUniformDescriptorSet(lightSBOsReferences, 1, mGlobalDescriptorSets[mFrameIndex].get());
}
void RenderSystem::Tick(float deltaTime)
{
    Prepare();
    // TickRotationMatrix();
    CollectEntities(); // Collect same material render entities
    // RenderShadowDepthPass();  // Shadow pass
    // void RenderDeferred();
    RenderForward();
    RenderSkyPass(); // Sky pass
    // RenderTranslucencyPass(); // Translucency pass
    // RenderPostProcessPass();  // Post process pass
    // RenderUIPass(deltaTime); // UI pass
    CopyColorAttachmentToSwapchainImage(*mRenderPassManager->GetRenderTargets()[mFrameIndex].colorImage);
    Present();
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
        HandleSwapchainOutOfDate();
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
void RenderSystem::RenderDeferred()
{
}
void RenderSystem::RenderForward()
{
    auto forwardFrameBuffers = mRenderPassManager->GetFrameBuffer(RenderPassType::ForwardComposition);
    auto renderTargetImages = mRenderPassManager->GetRenderTargets();
    auto extent = renderTargetImages[mFrameIndex].colorImage->GetExtent();
    vk::RenderPassBeginInfo renderPassBeginInfo;
    std::vector<vk::ClearValue> clearValues{
        vk::ClearValue(std::array<float, 4>{0.1f, 0.1f, 0.1f, 1.0f}), // 附件0: Render Target Color
        vk::ClearDepthStencilValue(1.0f, 0)                           // 附件1: Depth Stencil
    };
    renderPassBeginInfo.setClearValues(clearValues)
        .setRenderPass(mRenderPassManager->GetRenderPass(RenderPassType::ForwardComposition))
        .setFramebuffer(forwardFrameBuffers[mFrameIndex])
        .setRenderArea(vk::Rect2D({0, 0}, vk::Extent2D(extent.width, extent.height)));
    mGraphicCommandBuffers[mFrameIndex]->beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
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
    // subpass 0: 不透明物体
    // PBR
    {
        auto forwardOpaquePBRPipelineLayout = mPipelineLayoutManager->GetPipelineLayout(PipelineLayoutType::PBR);
        auto forwardOpaquePBRPipeline = mPipelineManager->GetPipeline(PipelineType::ForwardOpaquePBR);
        auto forwardOpaqueEntities = mRenderEntities[RenderType::ForwardOpaquePBR];
        // 1. 绑定管线
        mGraphicCommandBuffers[mFrameIndex]->bindPipeline(vk::PipelineBindPoint::eGraphics, forwardOpaquePBRPipeline);
        // 2. 绑定Global描述符集
        mGraphicCommandBuffers[mFrameIndex]->bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                                                forwardOpaquePBRPipelineLayout, 0,
                                                                mGlobalDescriptorSets[mFrameIndex].get(), {});
        for (auto entity : forwardOpaqueEntities)
        {
            auto &material = mRegistry->get<MaterialComponent>(entity);
            auto &mesh = mRegistry->get<MeshComponent>(entity);
            auto &transform = mRegistry->get<TransformComponent>(entity);
            // 1. 绑定push constant
            mGraphicCommandBuffers[mFrameIndex]->pushConstants(forwardOpaquePBRPipelineLayout,
                                                               vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4x4),
                                                               &transform.modelMatrix);
            // 3. 绑定材质描述符集
            auto materialDescriptorSet = material.material->GetDescriptorSet();
            mGraphicCommandBuffers[mFrameIndex]->bindDescriptorSets(
                vk::PipelineBindPoint::eGraphics, forwardOpaquePBRPipelineLayout, 1, materialDescriptorSet, {});
            // 4. 绑定顶点缓冲区
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
    {
    }
    mGraphicCommandBuffers[mFrameIndex]->nextSubpass(vk::SubpassContents::eInline);
    // subpass 1: 透明物体
    // PBR
    // Phong

    mGraphicCommandBuffers[mFrameIndex]->endRenderPass();
}
void RenderSystem::RenderTranslucencyPass()
{
    auto renderTargetImages = mRenderPassManager->GetRenderTargets();
    auto extent = renderTargetImages[mFrameIndex].colorImage->GetExtent();
    // PBR
    auto forwardTransparentPBREntities = mRenderEntities[RenderType::ForwardTransparentPBR];
    auto pipelineLayout = mPipelineLayoutManager->GetPipelineLayout(PipelineLayoutType::PBR);
    auto pipeline = mPipelineManager->GetPipeline(PipelineType::ForwardTransparentPBR);
    auto renderPass = mRenderPassManager->GetRenderPass(RenderPassType::Transparent);
    auto transparentFrameBuffers = mRenderPassManager->GetFrameBuffer(RenderPassType::Transparent);
    vk::RenderPassBeginInfo renderPassBeginInfo;
    std::vector<vk::ClearValue> clearValues{
        vk::ClearValue(std::array<float, 4>{0.1f, 0.1f, 0.1f, 1.0f}), // 附件0: Render Target
        vk::ClearDepthStencilValue(1.0f, 0)                           // 附件1: Depth Stencil
    };
    renderPassBeginInfo.setRenderPass(renderPass)
        .setFramebuffer(transparentFrameBuffers[mFrameIndex])
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
    mGraphicCommandBuffers[mFrameIndex]->endRenderPass();
}
void RenderSystem::RenderPostProcessPass()
{
}
void RenderSystem::RenderSkyPass()
{
    auto renderTargetImages = mRenderPassManager->GetRenderTargets();
    auto extent = renderTargetImages[mFrameIndex].colorImage->GetExtent();
    auto pipelineLayout = mPipelineLayoutManager->GetPipelineLayout(PipelineLayoutType::Skybox);
    auto pipeline = mPipelineManager->GetPipeline(PipelineType::Skybox);
    auto renderPass = mRenderPassManager->GetRenderPass(RenderPassType::Sky);
    auto skyFrameBuffers = mRenderPassManager->GetFrameBuffer(RenderPassType::Sky);
    vk::RenderPassBeginInfo renderPassBeginInfo;
    std::vector<vk::ClearValue> clearValues{
        vk::ClearValue(std::array<float, 4>{0.1f, 0.1f, 0.1f, 1.0f}), // 附件0: Render Target
        vk::ClearDepthStencilValue(1.0f, 0)                           // 附件1: Depth Stencil
    };
    renderPassBeginInfo.setRenderPass(renderPass)
        .setFramebuffer(skyFrameBuffers[mFrameIndex])
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
        // 2. 绑定Global描述符集
        mGraphicCommandBuffers[mFrameIndex]->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0,
                                                                mGlobalDescriptorSets[mFrameIndex].get(), {});

        // 6. 绘制
        mGraphicCommandBuffers[mFrameIndex]->draw(36, 1, 0, 0);
    }
    mGraphicCommandBuffers[mFrameIndex]->endRenderPass();
}
void RenderSystem::RenderUIPass(float deltaTime)
{
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
        HandleSwapchainOutOfDate();
    }
    mFrameIndex = (mFrameIndex + 1) % mFrameCount;
}
void RenderSystem::HandleSwapchainOutOfDate()
{
    mLogger->Info("Swapchain out of date, recreating swapchain");
    mContext->GetDevice().waitIdle();
    mContext->RecreateSwapchain();
    auto width = mContext->GetSurfaceInfo().extent.width;
    auto height = mContext->GetSurfaceInfo().extent.height;
    mRenderPassManager->RecreateRenderTargetFrameBuffer(width, height);
    InitialRenderTargetImageLayout();
    InitialSwapchainImageLayout();
}
void RenderSystem::CopyColorAttachmentToSwapchainImage(Image &image)
{
    // 渲染完成的图像转换为可传输布局
    vk::ImageMemoryBarrier lastFrameImagePreBarrier;
    lastFrameImagePreBarrier.setImage(image.GetHandle())
        .setOldLayout(vk::ImageLayout::eColorAttachmentOptimal)
        .setNewLayout(vk::ImageLayout::eTransferSrcOptimal)
        .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
        .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
        .setDstAccessMask(vk::AccessFlagBits::eTransferRead);
    mGraphicCommandBuffers[mFrameIndex]->pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                                         vk::PipelineStageFlagBits::eTransfer, {}, {}, {},
                                                         lastFrameImagePreBarrier);
    // swapchain图像转换为可传输布局
    vk::ImageMemoryBarrier swapchainPreBarrier;
    swapchainPreBarrier.setImage(mContext->GetSwapchainImages()[mImageIndex])
        .setOldLayout(vk::ImageLayout::ePresentSrcKHR)
        .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
        .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
        .setSrcAccessMask(vk::AccessFlagBits::eNone)
        .setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
    mGraphicCommandBuffers[mFrameIndex]->pipelineBarrier(
        vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, swapchainPreBarrier);

    // 拷贝渲染完成的图像到交换链图像
    auto swapchainExtent = mContext->GetSurfaceInfo().extent;
    auto renderTargetExtent = image.GetExtent();
    // auto offsetX = (swapchainExtent.width - renderTargetExtent.width) / 2;
    // auto offsetY = (swapchainExtent.height - renderTargetExtent.height) / 2;
    vk::ImageCopy imageCopy;
    imageCopy.setSrcSubresource(vk::ImageSubresourceLayers{vk::ImageAspectFlagBits::eColor, 0, 0, 1})
        .setSrcOffset({0, 0, 0})
        .setDstSubresource(vk::ImageSubresourceLayers{vk::ImageAspectFlagBits::eColor, 0, 0, 1})
        // .setDstOffset(vk::Offset3D{static_cast<int32_t>(offsetX), static_cast<int32_t>(offsetY), 0})
        .setDstOffset({0, 0, 0})
        .setExtent(renderTargetExtent);
    mGraphicCommandBuffers[mFrameIndex]->copyImage(image.GetHandle(), vk::ImageLayout::eTransferSrcOptimal,
                                                   mContext->GetSwapchainImages()[mImageIndex],
                                                   vk::ImageLayout::eTransferDstOptimal, imageCopy);
    // 交换链图像转换为可呈现布局
    vk::ImageMemoryBarrier swapchainPostBarrier;
    swapchainPostBarrier.setImage(mContext->GetSwapchainImages()[mImageIndex])
        .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
        .setNewLayout(vk::ImageLayout::ePresentSrcKHR)
        .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
        .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
        .setDstAccessMask(vk::AccessFlagBits::eNone);
    mGraphicCommandBuffers[mFrameIndex]->pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                                                         vk::PipelineStageFlagBits::eBottomOfPipe, {}, {}, {},
                                                         swapchainPostBarrier);
    // 将渲染的图像转回
    vk::ImageMemoryBarrier lastFrameImagePostBarrier;
    lastFrameImagePostBarrier.setImage(image.GetHandle())
        .setOldLayout(vk::ImageLayout::eTransferSrcOptimal)
        .setNewLayout(vk::ImageLayout::eColorAttachmentOptimal)
        .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
        .setSrcAccessMask(vk::AccessFlagBits::eTransferRead)
        .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
    mGraphicCommandBuffers[mFrameIndex]->pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                                                         vk::PipelineStageFlagBits::eColorAttachmentOutput, {}, {}, {},
                                                         lastFrameImagePostBarrier);
}
} // namespace MEngine