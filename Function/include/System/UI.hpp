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
    // ImGUI 相关
    ImGuiIO *mIO;
    ImGuiViewport *mMainViewport;
    ImGuiID mDockSpaceID;
    bool mIsFirstFrame = true;
    std::filesystem::path mProjectPath = std::filesystem::current_path(); // TODO：添加创建项目的功能，并修改此路径
    std::filesystem::path mAssetsPath = mProjectPath / "Assets";

  private:
    // Assets View
    std::filesystem::path mCurrentPath = mProjectPath;
    std::filesystem::path mUIResourcePath = std::filesystem::current_path() / "Resource" / "UI";
    float mIconSize = 64.0f; // 可调整的图标大小
    std::vector<UniqueImage> mIconImages;
    std::vector<vk::UniqueImageView> mIconImageViews;
    vk::DescriptorSet mFileIcon;
    vk::DescriptorSet mFolderIcon;
    vk::UniqueSampler mIconSampler;
    vk::UniqueCommandBuffer mIconTransitionCommandBuffer;
    vk::UniqueFence mIconTransitionFence;

  private:
    // Scene View
    bool mIsSceneViewPortChanged = false;
    uint32_t mSceneViewPortWidth = 0;
    uint32_t mSceneViewPortHeight = 0;
    vk::UniqueSampler mSceneSampler;

  private:
    entt::entity mCameraEntity;

  private:
    void SetDefaultWindowLayout();

    void DockingSpace();
    void HierarchyWindow();
    void InspectorWindow();
    void SceneViewWindow();
    void AssetWindow();

    void LoadUIIcon(const std::filesystem::path &iconPath, vk::DescriptorSet &descriptorSet);

    void RenderScene();

  public:
    UI(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context, std::shared_ptr<IWindow> window,
       std::shared_ptr<RenderPassManager> renderPassManager, std::shared_ptr<ImageFactory> imageFactory,
       std::shared_ptr<CommandBufferManager> commandBufferManager,
       std::shared_ptr<SyncPrimitiveManager> syncPrimitiveManager, std::shared_ptr<SamplerManager> samplerManager,
       std::shared_ptr<entt::registry> registry);
    ~UI();
    void ProcessEvent(const SDL_Event *event);
    void UpdateSceneDescriptorSet(vk::ImageView imageView, uint32_t imageIndex);
    void RecordUICommandBuffer(vk::CommandBuffer commandBuffer);

  public:
    inline bool IsSceneViewPortChanged() const
    {
        return mIsSceneViewPortChanged;
    }
    inline uint32_t GetSceneWidth() const
    {
        return mSceneViewPortWidth;
    }
    inline uint32_t GetSceneHeight() const
    {
        return mSceneViewPortHeight;
    }
};

} // namespace MEngine