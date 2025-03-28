#pragma once
#include "CommandBuffeManager.hpp"
#include "Componet/MaterialComponent.hpp"
#include "Componet/MeshComponent.hpp"
#include "Image.hpp"
#include "ImageManager.hpp"
#include "MEngine.hpp"
#include "PipelineLayoutManager.hpp"
#include "PipelineManager.hpp"
#include "RenderPassManager.hpp"
#include "ResourceManager.hpp"
#include "ShaderManager.hpp"
#include "SwapchainManager.hpp"
#include "SyncPrimitiveManager.hpp"
#include "TaskScheduler.hpp"
#include "Vertex.hpp"
#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"
#include <cstdint>
#include <memory>
#include <vector>

namespace MEngine
{

class RenderSystem
{
  private:
    std::shared_ptr<entt::registry> mRegistry;
    std::map<PipelineType, std::vector<entt::entity>> mBatchMaterialComponents;

    // std::vector<UniqueCommandBuffer> mPresnetCommandBuffers;
    std::unique_ptr<CommandBufferManager> mCommandBufferManager;
    std::unique_ptr<SyncPrimitiveManager> mSyncPrimitiveManager;
    std::unique_ptr<SwapchainManager> mSwapchainManager;
    std::unique_ptr<RenderPassManager> mRenderPassManager;
    std::unique_ptr<ImageManager> mImageManager;
    std::unique_ptr<PipelineLayoutManager> mPipelineLayoutManager;
    std::unique_ptr<PipelineManager> mPipelineManager;
    std::shared_ptr<ResourceManager> mResourceManager;
    std::unique_ptr<ShaderManager> mShaderManager;

    int64_t mFrameIndex;
    int64_t mFrameCount;
    std::vector<vk::UniqueSemaphore> mImageAvailableSemaphores;
    std::vector<vk::UniqueSemaphore> mRenderFinishedSemaphores;
    std::vector<vk::UniqueFence> mInFlightFences;

    uint32_t mImageIndex;
    vk::UniqueRenderPass mRenderPass;
    std::vector<vk::UniqueFramebuffer> mFrameBuffers;
    std::vector<UniqueImage> mDepthStencilImages;
    std::vector<vk::UniqueImageView> mDepthStencilImageViews;
    std::vector<std::vector<vk::UniqueCommandBuffer>> mSecondaryCommandBuffers;

    std::vector<UniqueCommandBuffer> mGraphicCommandBuffers;
    // pipeline and layout
    std::unordered_map<PipelineType, UniquePipeline> mPipelines;
    std::unordered_map<PipelineType, UniquePipelineLayout> mPipelineLayouts;

    void InitialPipeline();
    void CreateForwardOpaquePipeline();
    void CreateDeferredGBufferPipeline();
    void CreateShadowDepthPipeline();
    void CreatePostProcessPipeline();

  public:
    RenderSystem(std::shared_ptr<entt::registry> registry);
    void CollectRenderEntities();
    void Tick();
    void Prepare();
    void RenderForwardPass();
    void Present();
};
} // namespace MEngine