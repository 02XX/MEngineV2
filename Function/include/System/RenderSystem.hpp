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
#include "SyncPrimitiveManager.hpp"
#include "System.hpp"
#include "TaskScheduler.hpp"
#include "Vertex.hpp"
#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_vulkan.h"
#include <cstdint>
#include <memory>
#include <vector>
namespace MEngine
{

class RenderSystem : public System
{
  private:
    std::shared_ptr<entt::registry> mRegistry;
    std::map<PipelineType, std::vector<entt::entity>> mBatchMaterialComponents;

    std::shared_ptr<CommandBufferManager> mCommandBufferManager;
    std::shared_ptr<SyncPrimitiveManager> mSyncPrimitiveManager;
    std::shared_ptr<RenderPassManager> mRenderPassManager;
    std::shared_ptr<PipelineLayoutManager> mPipelineLayoutManager;
    std::shared_ptr<PipelineManager> mPipelineManager;

    int64_t mFrameIndex;
    int64_t mFrameCount;
    std::vector<vk::UniqueSemaphore> mImageAvailableSemaphores;
    std::vector<vk::UniqueSemaphore> mRenderFinishedSemaphores;
    std::vector<vk::UniqueFence> mInFlightFences;

    uint32_t mImageIndex;
    std::vector<std::vector<vk::UniqueCommandBuffer>> mSecondaryCommandBuffers;
    std::vector<vk::UniqueCommandBuffer> mGraphicCommandBuffers;

    // UI
    ImGuiIO *mIO;
    SDL_Window *mWindow;

  private:
    void Prepare();
    void RenderShadowDepthPass();
    void RenderDefferPass();
    void RenderTranslucencyPass();
    void RenderPostProcessPass();
    void RenderSkyPass();
    void RenderUIPass();
    void Present();

  public:
    RenderSystem(SDL_Window *window, std::shared_ptr<entt::registry> registry,
                 std::shared_ptr<CommandBufferManager> commandBufferManager,
                 std::shared_ptr<SyncPrimitiveManager> syncPrimitiveManager,
                 std::shared_ptr<RenderPassManager> renderPassManager,
                 std::shared_ptr<PipelineLayoutManager> pipelineLayoutManager,
                 std::shared_ptr<PipelineManager> pipelineManager);
    ~RenderSystem();
    void CollectRenderEntities();
    void Init() override;
    void Tick(float deltaTime) override;
    void Shutdown() override;
};
} // namespace MEngine