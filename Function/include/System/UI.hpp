#pragma once

#include "CommandBuffeManager.hpp"
#include "Context.hpp"
#include "Image.hpp"
#include "ImageFactory.hpp"
#include "Interface/ILogger.hpp"
#include "Interface/IWindow.hpp"
#include "RenderPassManager.hpp"
#include "SamplerManager.hpp"
#include "System/ISystem.hpp"
#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_vulkan.h"
#include "imgui_internal.h"

#include <glm/gtc/type_ptr.hpp>

#include "Componet/CameraComponent.hpp"
#include "ImGuizmo.h"
#include "ImageFactory.hpp"
#include "Math.hpp"
#include "stb_image.h"
#include <cstdint>
#include <filesystem>
#include <memory>
#include <vector>
#include <vulkan/vulkan_handles.hpp>

namespace MEngine
{
class UI
{
  private:
    std::shared_ptr<ILogger> mLogger;
    std::shared_ptr<Context> mContext;
    std::shared_ptr<IWindow> mWindow;
    std::shared_ptr<RenderPassManager> mRenderPassManager;
    std::shared_ptr<ImageFactory> mImageFactory;
    std::shared_ptr<CommandBufferManager> mCommandBufferManager;
    std::shared_ptr<SyncPrimitiveManager> mSyncPrimitiveManager;
    std::shared_ptr<SamplerManager> mSamplerManager;
    std::shared_ptr<entt::registry> mRegistry;

  private:
    vk::UniqueDescriptorPool mUIDescriptorPool;
    ImGuiIO *mIO;

    uint32_t mCurrentFrame = 0;
    bool mFirstRun = true;

    uint32_t mSceneWidth;
    uint32_t mSceneHeight;
    vk::UniqueDescriptorSetLayout mSceneDescriptorSetLayout;
    std::vector<vk::UniqueDescriptorSet> mSceneDescriptorSets;
    vk::UniqueSampler mSceneSampler;
    vk::UniqueCommandBuffer mImageTransitionCommandBuffer;
    vk::UniqueFence mImageTransitionFence;

    bool mIsSceneViewPortChange = false;

    std::filesystem::path mCurrentAssetDir = std::filesystem::current_path();
    float mThumbnailSize = 64.0f; // 可调整的图标大小

    std::filesystem::path mAssetsPath = std::filesystem::current_path() / "Assets";
    UniqueImage mFileImage;
    vk::UniqueImageView mFileImageView;
    UniqueImage mFolderImage;
    vk::UniqueImageView mFolderImageView;
    vk::UniqueSampler mAssetSampler;
    vk::DescriptorSet mFileTexture;
    vk::DescriptorSet mFolderTexture;

    entt::entity mCameraEntity;

  private:
    void DockingSpace();
    void HierarchyWindow();
    void InspectorWindow();
    void SceneViewWindow();
    void AssetWindow();

    void CreateDescriptorPool();
    void CreateSceneDescriptorSetLayout();
    void CreateSceneDescriptorSet();
    void CreateSampler();

    void LoadAsset();

    void RenderScene();

  public:
    UI(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context, std::shared_ptr<IWindow> window,
       std::shared_ptr<RenderPassManager> renderPassManager, std::shared_ptr<ImageFactory> imageFactory,
       std::shared_ptr<CommandBufferManager> commandBufferManager,
       std::shared_ptr<SyncPrimitiveManager> syncPrimitiveManager, std::shared_ptr<SamplerManager> samplerManager,
       std::shared_ptr<entt::registry> registry);
    ~UI();

    bool IsSceneViewPortChanged() const
    {
        return mIsSceneViewPortChange;
    }
    uint32_t GetSceneWidth() const
    {
        return mSceneWidth;
    }
    uint32_t GetSceneHeight() const
    {
        return mSceneHeight;
    }
    void SetCamera(entt::entity cameraEntity)
    {
        mCameraEntity = cameraEntity;
    }

    void ProcessEvent(const SDL_Event *event);
    void UpdateSceneDescriptorSet(vk::ImageView imageView, uint32_t imageIndex);
    void RecordUICommandBuffer(vk::CommandBuffer commandBuffer);
};

} // namespace MEngine