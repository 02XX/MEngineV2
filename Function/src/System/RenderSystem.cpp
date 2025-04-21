#include "System/RenderSystem.hpp"
#include "Entity/PBRMaterial.hpp"
#include <vector>
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
    mUI->RenderUI();
    CollectRenderEntities(); // Collect same material render entities
    CollectMainCamera();
    // RenderShadowDepthPass();  // Shadow pass
    void RenderDeferred(); // Deferred pass
    // RenderSkyPass();          // Sky pass
    // RenderTranslucencyPass(); // Translucency pass
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
    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    mGraphicCommandBuffers[mFrameIndex]->begin(beginInfo);
}
void RenderSystem::RenderShadowDepthPass()
{
}
void RenderSystem::RenderDeferred()
{
    auto extent = mRenderPassManager->GetExtent();
    auto deferredEntities = mRenderEntities[RenderType::Deferred];
    auto deferredFrameResources = mRenderPassManager->GetDeferredCompositionFrameResource();
    auto deferredRenderPass = mRenderPassManager->GetRenderPass(RenderPassType::DeferredComposition);
    auto deferredFrameBuffers = mRenderPassManager->GetFrameBuffer(RenderPassType::DeferredComposition);

    vk::RenderPassBeginInfo deferredRenderPassBeginInfo;
    std::vector<vk::ClearValue> clearValues{
        vk::ClearValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}), // 附件0: Color
        vk::ClearValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}), // 附件1: 位置
        vk::ClearValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}), // 附件2: 法线
        vk::ClearValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}), // 附件3: Albedo
        vk::ClearValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}), // 附件4: 金属/粗糙度
        vk::ClearValue(std::array<float, 4>{1.0f, 1.0f, 1.0f, 1.0f}), // 附件5: AO
        vk::ClearDepthStencilValue(1.0f, 0)                           // 附件6: 深度
    };
    deferredRenderPassBeginInfo.setRenderPass(deferredRenderPass)
        .setFramebuffer(deferredFrameBuffers[mImageIndex])
        .setRenderArea(vk::Rect2D({0, 0}, extent))
        .setClearValues(clearValues);
    mGraphicCommandBuffers[mFrameIndex]->beginRenderPass(deferredRenderPassBeginInfo, vk::SubpassContents::eInline);
    {
        // 设置视口和裁剪
        vk::Viewport viewport;
        viewport.setX(0.0f)
            .setY(0.0f)
            .setWidth(static_cast<float>(extent.width))
            .setHeight(static_cast<float>(extent.height))
            .setMinDepth(0.0f)
            .setMaxDepth(1.0f);
        mGraphicCommandBuffers[mFrameIndex]->setViewport(0, viewport);
        vk::Rect2D scissor;
        scissor.setOffset({0, 0}).setExtent(extent);
        mGraphicCommandBuffers[mFrameIndex]->setScissor(0, scissor);
        // subpass: 0 GBuffer
        auto gBufferPipelineLayout = mPipelineLayoutManager->GetPipelineLayout(PipelineLayoutType::PBR);
        auto gBufferPipeline = mPipelineManager->GetPipeline(PipelineType::DeferredGBuffer);
        for (auto entity : deferredEntities)
        {
            // 1. 绑定管线
            mGraphicCommandBuffers[mFrameIndex]->bindPipeline(vk::PipelineBindPoint::eGraphics, gBufferPipeline);
            auto &materialComponent = mRegistry->get<MaterialComponent>(entity);
            auto &meshComponent = mRegistry->get<MeshComponent>(entity);
            auto &transform = mRegistry->get<TransformComponent>(entity);
            // 2. 绑定push constant
            mGraphicCommandBuffers[mFrameIndex]->pushConstants(gBufferPipelineLayout, vk::ShaderStageFlagBits::eVertex,
                                                               0, sizeof(glm::mat4x4), &transform.modelMatrix);
            // 3. 绑定描述符集
            mGraphicCommandBuffers[mFrameIndex]->bindDescriptorSets(
                vk::PipelineBindPoint::eGraphics, gBufferPipelineLayout, 0,
                {mGlobalDescriptorSets[mFrameIndex].get(), materialComponent.material->GetDescriptorSet()}, {});
            // 4. 绑定缓冲区
            auto vertexBuffer = meshComponent.mesh->GetVertexBuffer();
            auto indexBuffer = meshComponent.mesh->GetIndexBuffer();
            auto indexCount = meshComponent.mesh->GetIndexCount();
            mGraphicCommandBuffers[mFrameIndex]->bindVertexBuffers(0, {vertexBuffer}, {0});
            mGraphicCommandBuffers[mFrameIndex]->bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint32);
            // 5. 绘制
            mGraphicCommandBuffers[mFrameIndex]->drawIndexed(indexCount, 1, 0, 0, 0);
            // 6. 转换布局
            // albedo
            vk::ImageMemoryBarrier albedoBarrier;
            albedoBarrier.setImage(deferredFrameResources[mFrameIndex]->albedoImage->GetHandle())
                .setOldLayout(vk::ImageLayout::eUndefined)
                .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                .setSubresourceRange(vk::ImageSubresourceRange()
                                         .setAspectMask(vk::ImageAspectFlagBits::eColor)
                                         .setBaseMipLevel(0)
                                         .setLevelCount(1)
                                         .setBaseArrayLayer(0)
                                         .setLayerCount(1))
                .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
                .setDstAccessMask(vk::AccessFlagBits::eShaderRead);
            mGraphicCommandBuffers[mFrameIndex]->pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                                                 vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {},
                                                                 albedoBarrier);
            // position
            vk::ImageMemoryBarrier positionBarrier;
            positionBarrier.setImage(deferredFrameResources[mFrameIndex]->positionImage->GetHandle())
                .setOldLayout(vk::ImageLayout::eUndefined)
                .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                .setSubresourceRange(vk::ImageSubresourceRange()
                                         .setAspectMask(vk::ImageAspectFlagBits::eColor)
                                         .setBaseMipLevel(0)
                                         .setLevelCount(1)
                                         .setBaseArrayLayer(0)
                                         .setLayerCount(1))
                .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
                .setDstAccessMask(vk::AccessFlagBits::eShaderRead);
            mGraphicCommandBuffers[mFrameIndex]->pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                                                 vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {},
                                                                 positionBarrier);
            // normal
            vk::ImageMemoryBarrier normalBarrier;
            normalBarrier.setImage(deferredFrameResources[mFrameIndex]->normalImage->GetHandle())
                .setOldLayout(vk::ImageLayout::eUndefined)
                .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                .setSubresourceRange(vk::ImageSubresourceRange()
                                         .setAspectMask(vk::ImageAspectFlagBits::eColor)
                                         .setBaseMipLevel(0)
                                         .setLevelCount(1)
                                         .setBaseArrayLayer(0)
                                         .setLayerCount(1))
                .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
                .setDstAccessMask(vk::AccessFlagBits::eShaderRead);
            mGraphicCommandBuffers[mFrameIndex]->pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                                                 vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {},
                                                                 normalBarrier);
            // metallicRoughness
            vk::ImageMemoryBarrier metallicRoughnessBarrier;
            metallicRoughnessBarrier.setImage(deferredFrameResources[mFrameIndex]->metallicRoughnessImage->GetHandle())
                .setOldLayout(vk::ImageLayout::eUndefined)
                .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                .setSubresourceRange(vk::ImageSubresourceRange()
                                         .setAspectMask(vk::ImageAspectFlagBits::eColor)
                                         .setBaseMipLevel(0)
                                         .setLevelCount(1)
                                         .setBaseArrayLayer(0)
                                         .setLayerCount(1))
                .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
                .setDstAccessMask(vk::AccessFlagBits::eShaderRead);
            mGraphicCommandBuffers[mFrameIndex]->pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                                                 vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {},
                                                                 metallicRoughnessBarrier);
            // ao
            vk::ImageMemoryBarrier aoBarrier;
            aoBarrier.setImage(deferredFrameResources[mFrameIndex]->aoImage->GetHandle())
                .setOldLayout(vk::ImageLayout::eUndefined)
                .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                .setSubresourceRange(vk::ImageSubresourceRange()
                                         .setAspectMask(vk::ImageAspectFlagBits::eColor)
                                         .setBaseMipLevel(0)
                                         .setLevelCount(1)
                                         .setBaseArrayLayer(0)
                                         .setLayerCount(1))
                .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
                .setDstAccessMask(vk::AccessFlagBits::eShaderRead);
            mGraphicCommandBuffers[mFrameIndex]->pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                                                 vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {},
                                                                 aoBarrier);
            // emissive
            vk::ImageMemoryBarrier emissiveBarrier;
            emissiveBarrier.setImage(deferredFrameResources[mFrameIndex]->emissiveImage->GetHandle())
                .setOldLayout(vk::ImageLayout::eUndefined)
                .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                .setSubresourceRange(vk::ImageSubresourceRange()
                                         .setAspectMask(vk::ImageAspectFlagBits::eColor)
                                         .setBaseMipLevel(0)
                                         .setLevelCount(1)
                                         .setBaseArrayLayer(0)
                                         .setLayerCount(1))
                .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
                .setDstAccessMask(vk::AccessFlagBits::eShaderRead);
            mGraphicCommandBuffers[mFrameIndex]->pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                                                 vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {},
                                                                 emissiveBarrier);
            // subpass: 1 Lighting
            mGraphicCommandBuffers[mFrameIndex]->nextSubpass(vk::SubpassContents::eInline);
            // 1. 绑定管线
            auto lightingPipelineLayout =
                mPipelineLayoutManager->GetPipelineLayout(PipelineLayoutType::DeferredLighting);
            auto lightingPipeline = mPipelineManager->GetPipeline(PipelineType::DeferredLighting);
            mGraphicCommandBuffers[mFrameIndex]->bindPipeline(vk::PipelineBindPoint::eGraphics, lightingPipeline);
            // 2. 为GBuffer更新描述符集
            auto lightingDescriptorSet =
                static_cast<PBRMaterial *>(materialComponent.material)->GetLightingDescriptorSet();
            vk::DescriptorImageInfo albedoImageInfo;
            albedoImageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                .setImageView(deferredFrameResources[mFrameIndex]->albedoImageView.get())
                .setSampler(nullptr);
            vk::WriteDescriptorSet albedoWriter;
            albedoWriter.setImageInfo(albedoImageInfo)
                .setDescriptorType(vk::DescriptorType::eInputAttachment)
                .setDstBinding(0)
                .setDstSet(lightingDescriptorSet);
            vk::DescriptorImageInfo positionImageInfo;
            positionImageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                .setImageView(deferredFrameResources[mFrameIndex]->positionImageView.get())
                .setSampler(nullptr);
            vk::WriteDescriptorSet positionWriter;
            positionWriter.setImageInfo(positionImageInfo)
                .setDescriptorType(vk::DescriptorType::eInputAttachment)
                .setDstBinding(1)
                .setDstSet(lightingDescriptorSet);
            vk::DescriptorImageInfo normalImageInfo;
            normalImageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                .setImageView(deferredFrameResources[mFrameIndex]->normalImageView.get())
                .setSampler(nullptr);
            vk::WriteDescriptorSet normalWriter;
            normalWriter.setImageInfo(normalImageInfo)
                .setDescriptorType(vk::DescriptorType::eInputAttachment)
                .setDstBinding(2)
                .setDstSet(lightingDescriptorSet);
            vk::DescriptorImageInfo metallicRoughnessImageInfo;
            metallicRoughnessImageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                .setImageView(deferredFrameResources[mFrameIndex]->metallicRoughnessImageView.get())
                .setSampler(nullptr);
            vk::WriteDescriptorSet metallicRoughnessWriter;
            metallicRoughnessWriter.setImageInfo(metallicRoughnessImageInfo)
                .setDescriptorType(vk::DescriptorType::eInputAttachment)
                .setDstBinding(3)
                .setDstSet(lightingDescriptorSet);
            vk::DescriptorImageInfo aoImageInfo;
            aoImageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                .setImageView(deferredFrameResources[mFrameIndex]->aoImageView.get())
                .setSampler(nullptr);
            vk::WriteDescriptorSet aoWriter;
            aoWriter.setImageInfo(aoImageInfo)
                .setDescriptorType(vk::DescriptorType::eInputAttachment)
                .setDstBinding(4)
                .setDstSet(lightingDescriptorSet);
            vk::DescriptorImageInfo emissiveImageInfo;
            emissiveImageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                .setImageView(deferredFrameResources[mFrameIndex]->emissiveImageView.get())
                .setSampler(nullptr);
            vk::WriteDescriptorSet emissiveWriter;
            emissiveWriter.setImageInfo(emissiveImageInfo)
                .setDescriptorType(vk::DescriptorType::eInputAttachment)
                .setDstBinding(5)
                .setDstSet(lightingDescriptorSet);
            std::vector<vk::WriteDescriptorSet> writers{
                albedoWriter, positionWriter, normalWriter, metallicRoughnessWriter, aoWriter, emissiveWriter};
            mContext->GetDevice().updateDescriptorSets(writers, {});
            // 3. 绑定描述符集
            mGraphicCommandBuffers[mFrameIndex]->bindDescriptorSets(
                vk::PipelineBindPoint::eGraphics, lightingPipelineLayout, 0,
                {mGlobalDescriptorSets[mFrameIndex].get(), lightingDescriptorSet}, {});
            //  3. 绘制 全屏四边形
            mGraphicCommandBuffers[mFrameIndex]->drawIndexed(3, 1, 0, 0, 0);
        }
    }
    mGraphicCommandBuffers[mFrameIndex]->endRenderPass();
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
    auto extent = mRenderPassManager->GetExtent();
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
        for (auto entity : forwardTransparentPBREntities)
        {
            auto &material = mRegistry->get<MaterialComponent>(entity);
            auto &mesh = mRegistry->get<MeshComponent>(entity);
            auto &transform = mRegistry->get<TransformComponent>(entity);
            // 2. 绑定push constant
            mGraphicCommandBuffers[mFrameIndex]->pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0,
                                                               sizeof(glm::mat4x4), &transform.modelMatrix);
            // 3. 绑定Global描述符集
            mGraphicCommandBuffers[mFrameIndex]->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0,
                                                                    mGlobalDescriptorSets[mFrameIndex].get(), {});
            // 4. 绑定材质描述符集
            auto materialDescriptorSet = material.material->GetDescriptorSet();
            mGraphicCommandBuffers[mFrameIndex]->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 1,
                                                                    materialDescriptorSet, {});
            //  5. 绑定顶点缓冲区
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