#pragma once

#include "Context.hpp"
#include "Image.hpp"
#include "Interface/ILogger.hpp"
#include "Interface/IWindow.hpp"
#include "RenderPassManager.hpp"
#include "System/ISystem.hpp"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_vulkan.h"
#include "imgui_internal.h"
#include <cstdint>
#include <memory>
#include <vector>
#include <vulkan/vulkan_handles.hpp>

namespace MEngine
{
class UISystem : public ISystem
{
  private:
    std::shared_ptr<ILogger> mLogger;
    std::shared_ptr<Context> mContext;
    std::shared_ptr<IWindow> mWindow;
    std::shared_ptr<RenderPassManager> mRenderPassManager;
    std::shared_ptr<ImageManager> mImageManager;

  private:
    vk::UniqueDescriptorPool mUIDescriptorPool;
    ImGuiIO *mIO;
    vk::CommandBuffer mCommandBuffer;
    uint32_t mCurrentFrame = 0;
    bool mFirstRun = true;

    uint32_t mSceneWidth = 1280;
    uint32_t mSceneHeight = 720;
    std::vector<UniqueImage> mSceneImages;
    vk::UniqueDescriptorSetLayout mSceneDescriptorSetLayout;
    std::vector<vk::UniqueDescriptorSet> mSceneDescriptorSets;
    std::vector<vk::UniqueImageView> mImageSceneViews;
    std::vector<vk::UniqueFramebuffer> mSceneFramebuffers;
    vk::UniqueRenderPass mSceneRenderPasses;
    vk::UniqueSampler mSceneSampler;

  private:
    void DockingSpace();
    void HierarchyWindow();
    void InspectorWindow();
    void SceneViewWindow();
    void AssetWindow();

    void CreateSceneRenderPass();
    void CreateSceneFrameBuffer();
    void CreateSceneImage();
    void CreateSceneImageView();
    void CreateDescriptorPool();
    void CreateSceneDescriptorSetLayout();
    void CreateSceneDescriptorSet();
    void CreateSampler();
    void UpdateSceneDescriptorSet();

    void RenderScene();

  public:
    UISystem(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context, std::shared_ptr<IWindow> window,
             std::shared_ptr<RenderPassManager> renderPassManager, std::shared_ptr<ImageManager> mImageManager);
    void SetCommandBuffer(vk::CommandBuffer commandBuffer)
    {
        mCommandBuffer = commandBuffer;
    }
    void SetCurrentFrame(uint32_t currentFrame)
    {
        mCurrentFrame = currentFrame;
    }
    void ProcessEvent(const SDL_Event *event);
    void Init() override;
    void Tick(float deltaTime) override;
    void Shutdown() override;
};

} // namespace MEngine