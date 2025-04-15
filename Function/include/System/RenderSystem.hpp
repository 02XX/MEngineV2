#pragma once
#include "Buffer.hpp"
#include "BufferFactory.hpp"
#include "CommandBuffeManager.hpp"
#include "Componet/CameraComponent.hpp"
#include "Componet/MaterialComponent.hpp"
#include "Componet/MeshComponent.hpp"
#include "Componet/TransformComponent.hpp"
#include "Context.hpp"
#include "DescriptorManager.hpp"
#include "Entity/Interface/IMaterial.hpp"
#include "Image.hpp"
#include "ImageFactory.hpp"
#include "Interface/ILogger.hpp"
#include "Interface/IWindow.hpp"
#include "MEngine.hpp"
#include "Math.hpp"
#include "PipelineLayoutManager.hpp"
#include "PipelineManager.hpp"
#include "RenderPassManager.hpp"
#include "ResourceManager.hpp"
#include "SamplerManager.hpp"
#include "ShaderManager.hpp"
#include "SyncPrimitiveManager.hpp"
#include "System.hpp"
#include "System/UI.hpp"
#include "TaskScheduler.hpp"
#include "Vertex.hpp"
#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"
#include <cstdint>
#include <memory>
#include <vector>
namespace MEngine
{

class RenderSystem final : public System
{
  private:
    // DI

    std::shared_ptr<RenderPassManager> mRenderPassManager;
    std::shared_ptr<PipelineLayoutManager> mPipelineLayoutManager;
    std::shared_ptr<PipelineManager> mPipelineManager;

    std::shared_ptr<CommandBufferManager> mCommandBufferManager;
    std::shared_ptr<SyncPrimitiveManager> mSyncPrimitiveManager;
    std::shared_ptr<DescriptorManager> mDescriptorManager;
    std::shared_ptr<SamplerManager> mSamplerManager;

    std::shared_ptr<BufferFactory> mBufferFactory;
    std::shared_ptr<ImageFactory> mImageFactory;

    std::shared_ptr<IWindow> mWindow;
    std::shared_ptr<UI> mUI;

  private:
    std::map<RenderType, std::vector<entt::entity>> mRenderEntities;

    int64_t mFrameIndex;
    int64_t mFrameCount;
    std::vector<vk::UniqueSemaphore> mImageAvailableSemaphores;
    std::vector<vk::UniqueSemaphore> mRenderFinishedSemaphores;
    std::vector<vk::UniqueFence> mInFlightFences;
    vk::UniqueFence mUIUpdateFence;

    uint32_t mImageIndex;
    std::vector<std::vector<vk::UniqueCommandBuffer>> mSecondaryCommandBuffers;
    std::vector<vk::UniqueCommandBuffer> mGraphicCommandBuffers;

    // Global DescriptorSet
    std::vector<vk::UniqueDescriptorSet> mGlobalDescriptorSets;
    // main camera
    entt::entity mMainCameraEntity;
    UniqueBuffer mVPUBO;
    struct VPUniform
    {
        glm::mat4 view;
        glm::mat4 projection;
    } mVPUniform;

  private:
    void CollectRenderEntities();
    void CollectMainCamera();
    void Prepare();
    void RenderShadowDepthPass();
    void RenderDeferred();
    void RenderForward();
    void RenderSkyPass();
    void RenderTranslucencyPass();
    void RenderPostProcessPass();
    void RenderUIPass(float deltaTime);
    void Present();

  public:
    RenderSystem(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                 std::shared_ptr<IConfigure> configure, std::shared_ptr<entt::registry> registry,
                 std::shared_ptr<RenderPassManager> renderPassManager,
                 std::shared_ptr<PipelineLayoutManager> pipelineLayoutManager,
                 std::shared_ptr<PipelineManager> pipelineManager,
                 std::shared_ptr<CommandBufferManager> commandBufferManager,
                 std::shared_ptr<SyncPrimitiveManager> syncPrimitiveManager,
                 std::shared_ptr<DescriptorManager> descriptorManager, std::shared_ptr<SamplerManager> samplerManager,
                 std::shared_ptr<BufferFactory> bufferFactory, std::shared_ptr<ImageFactory> imageFactory,
                 std::shared_ptr<IWindow> window, std::shared_ptr<UI> ui);
    ~RenderSystem() override;
    void Init() override;
    void Tick(float deltaTime) override;
    void Shutdown() override;
};
} // namespace MEngine