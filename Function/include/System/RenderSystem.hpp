#pragma once
#include "Buffer.hpp"
#include "BufferFactory.hpp"
#include "CommandBuffeManager.hpp"
#include "Component/CameraComponent.hpp"
#include "Component/LightComponent.hpp"
#include "Component/MaterialComponent.hpp"
#include "Component/MeshComponent.hpp"
#include "Component/TransformComponent.hpp"
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
#include "ShaderManager.hpp"
#include "SyncPrimitiveManager.hpp"
#include "System.hpp"
#include "TaskScheduler.hpp"
#include "Vertex.hpp"
#include "entt/entt.hpp"
#include <cstdint>
#include <memory>
#include <vector>

namespace MEngine
{
class RenderSystem : public System
{
  protected:
    // DI
    std::shared_ptr<RenderPassManager> mRenderPassManager;
    std::shared_ptr<PipelineLayoutManager> mPipelineLayoutManager;
    std::shared_ptr<PipelineManager> mPipelineManager;

    std::shared_ptr<CommandBufferManager> mCommandBufferManager;
    std::shared_ptr<SyncPrimitiveManager> mSyncPrimitiveManager;
    std::shared_ptr<DescriptorManager> mDescriptorManager;

    std::shared_ptr<BufferFactory> mBufferFactory;
    std::shared_ptr<ImageFactory> mImageFactory;

    std::shared_ptr<IWindow> mWindow;

  protected:
    std::map<RenderType, std::vector<entt::entity>> mRenderEntities;

    std::vector<vk::UniqueSemaphore> mImageAvailableSemaphores;
    std::vector<vk::UniqueSemaphore> mRenderFinishedSemaphores;
    std::vector<vk::UniqueFence> mInFlightFences;

    uint32_t mFrameIndex;
    uint32_t mFrameCount;
    uint32_t mImageIndex;
    std::vector<std::vector<vk::UniqueCommandBuffer>> mSecondaryCommandBuffers;
    std::vector<vk::UniqueCommandBuffer> mGraphicCommandBuffers;

    // Global DescriptorSet
    std::vector<vk::UniqueDescriptorSet> mGlobalDescriptorSets;
    // Camera_UBO
    UniqueBuffer mCameraUBO;
    struct CameraUniform
    {
        glm::mat4 view;
        glm::mat4 projection;
        glm::vec3 position; // 位置
    } mCameraUniform;
    // Light_SBO[6]
    std::array<UniqueBuffer, 6> mLightSBOs;
    struct LightUniform
    {
        glm::vec3 position; // 位置
        float range;        // 范围

        glm::vec3 direction; // 方向
        float coneAngle;     // 锥角 only for spot light

        glm::vec3 color; // 颜色
        float intensity; // 强度

        LightType type;
    };
    // ShadowParameters_SBO
    //  main camera
    entt::entity mMainCameraEntity;

  protected:
    void InitialRenderTargetImageLayout();
    void InitialSwapchainImageLayout();
    void CollectEntities();
    void Prepare();
    void RenderShadowDepthPass();
    void RenderDeferred();
    void RenderForward();
    void RenderSkyPass();
    void RenderTranslucencyPass();
    void RenderPostProcessPass();
    void RenderUIPass(float deltaTime);
    void Present();

    virtual void HandleSwapchainOutOfDate();
    virtual void CopyColorAttachmentToSwapchainImage(Image &image);

  public:
    RenderSystem(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                 std::shared_ptr<IConfigure> configure, std::shared_ptr<entt::registry> registry,
                 std::shared_ptr<RenderPassManager> renderPassManager,
                 std::shared_ptr<PipelineLayoutManager> pipelineLayoutManager,
                 std::shared_ptr<PipelineManager> pipelineManager,
                 std::shared_ptr<CommandBufferManager> commandBufferManager,
                 std::shared_ptr<SyncPrimitiveManager> syncPrimitiveManager,
                 std::shared_ptr<DescriptorManager> descriptorManager, std::shared_ptr<BufferFactory> bufferFactory,
                 std::shared_ptr<ImageFactory> imageFactory);
    ~RenderSystem();
    inline auto BeginRender()
    {
        Prepare();
    }
    inline auto EndRender()
    {
        Present();
    }
    virtual void Init() override;
    virtual void Tick(float deltaTime) override;
    virtual void Shutdown() override;
};
} // namespace MEngine