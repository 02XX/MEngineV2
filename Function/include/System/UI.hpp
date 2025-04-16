#pragma once
#include "CommandBuffeManager.hpp"
#include "Context.hpp"
#include "Entity/PBRMaterial.hpp"
#include "Entity/Texture.hpp"
#include "Image.hpp"
#include "ImageFactory.hpp"
#include "Interface/ILogger.hpp"
#include "Interface/IWindow.hpp"
#include "RenderPassManager.hpp"
#include "SamplerManager.hpp"
#include "entt/entt.hpp"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_vulkan.h"
#include "imgui_internal.h"


#include <cstdint>
#include <memory>
#include <vector>

#include "Component/AssestComponent.hpp"
#include "Component/CameraComponent.hpp"
#include "Component/MaterialComponent.hpp"
#include "Component/MeshComponent.hpp"
#include "Component/TransformComponent.hpp"
#include "Entity/Interface/IMaterial.hpp"
#include "Entity/Interface/ITexture.hpp"
#include "ImGuizmo.h"
#include "ImageFactory.hpp"
#include "Math.hpp"
#include "PipelineManager.hpp"
#include "Repository/Interface/IRepository.hpp"
#include "stb_image.h"
#include <algorithm>
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
    std::shared_ptr<IRepository<PBRMaterial>> mPBRMaterialRepository;
    std::shared_ptr<IRepository<Texture>> mTextureRepository;

  private:
    // ImGUI 相关
    ImGuiIO *mIO;
    ImGuiViewport *mMainViewport;
    ImGuiID mDockSpaceID;
    bool mIsFirstFrame = true;
    std::filesystem::path mProjectPath = std::filesystem::current_path(); // TODO：添加创建项目的功能，并修改此路径
    std::filesystem::path mAssetsPath = mProjectPath / "Assets";
    ImFont *mMSYHFont; // 微软雅黑字体
    // ImGUIZmo 相关
    float mGizmoWidth = 10.f;
    float mGizmoHeight = 10.f;
    ImGuizmo::OPERATION mGuizmoOperation = ImGuizmo::TRANSLATE;
    ImGuizmo::MODE mGuizmoMode = ImGuizmo::LOCAL;

  private:
    // Assets View
    std::shared_ptr<entt::registry> mAssetRegistry;
    std::vector<entt::entity> mProcessedAssets;
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
    entt::entity mAssetsSelectedEntity = entt::null;
    entt::entity mAssetsHoveredEntity = entt::null;

  private:
    // Toolbar View
    bool mIsPlay = false;
    bool mIsPause = false;
    bool mIsStop = true;

  private:
    // Scene View
    bool mIsSceneViewPortChanged = false;
    uint32_t mSceneViewPortWidth = 0;
    uint32_t mSceneViewPortHeight = 0;
    vk::UniqueSampler mSceneSampler;
    std::vector<vk::DescriptorSet> mSceneDescriptorSets;
    uint32_t mImageIndex = 0;

  private:
    // Inspector View
    uint32_t mInspectorImageWidth = 50;
    uint32_t mInspectorImageHeight = 50;
    vk::DescriptorSet mDefaultTextureDescriptorSet;
    void InspectorMaterial(MaterialComponent &materialComponent);

  private:
    entt::entity mMainCamera;

    entt::entity mSelectedEntity = entt::null;
    entt::entity mHoveredEntity = entt::null;

  private:
    void SetDefaultWindowLayout();
    void RightClickMenu();
    void DockingSpace();
    void HierarchyWindow();
    void InspectorWindow();
    void ToolbarWindow();
    void SceneViewWindow();
    void AssetWindow();
    void EntryFolder(const std::filesystem::path &path);
    void UpdateAssetsView();
    void OnAssetCreated(entt::registry &, entt::entity entity);
    void OnAssetUpdated(entt::registry &, entt::entity);
    void OnAssetDestroyed(entt::registry &, entt::entity entity);
    void ProcessAssets();
    void DisplayAssetEntity();
    void LoadUIIcon(const std::filesystem::path &iconPath, vk::DescriptorSet &descriptorSet);

    void CollectEntity();

  private:
    void ShowTexture(const std::string &name, std::shared_ptr<Texture> texture = nullptr, ImVec2 size = ImVec2(50, 50));

  public:
    UI(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context, std::shared_ptr<IWindow> window,
       std::shared_ptr<RenderPassManager> renderPassManager, std::shared_ptr<ImageFactory> imageFactory,
       std::shared_ptr<CommandBufferManager> commandBufferManager,
       std::shared_ptr<SyncPrimitiveManager> syncPrimitiveManager, std::shared_ptr<SamplerManager> samplerManager,
       std::shared_ptr<entt::registry> registry, std::shared_ptr<IRepository<PBRMaterial>> pbrMaterialRepository,
       std::shared_ptr<IRepository<Texture>> textureRepository);
    ~UI();
    void ProcessEvent(const SDL_Event *event);
    void RenderUI();
    void RecordUICommandBuffer(vk::CommandBuffer commandBuffer);
    void SetSceneViewPort();

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
    inline void SetImageIndex(uint32_t imageIndex)
    {
        mImageIndex = imageIndex;
    }
};

} // namespace MEngine