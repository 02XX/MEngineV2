#pragma once
#include "CommandBuffeManager.hpp"
#include "Context.hpp"
#include "Entity/PBRMaterial.hpp"
#include "Entity/Texture2D.hpp"
#include "Image.hpp"
#include "ImageFactory.hpp"
#include "Interface/IConfigure.hpp"
#include "Interface/ILogger.hpp"
#include "Interface/IWindow.hpp"
#include "RenderPassManager.hpp"
#include "ResourceManager.hpp"
#include "SamplerManager.hpp"
#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_vulkan.h"
#include "imgui_internal.h"
#include <algorithm>
#include <cstdint>
#include <memory>
#include <stack>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "Component/AssestComponent.hpp"
#include "Component/CameraComponent.hpp"
#include "Component/InputComponent.hpp"
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
#include "System/RenderSystem.hpp"
#include "System/System.hpp"
#include "stb_image.h"

#undef max
namespace MEngine
{
// class FileExplorer
// {
//   private:
//     std::shared_ptr<ResourceManager> mResourceManager;
//     std::stack<std::filesystem::path> mBackStack;
//     std::stack<std::filesystem::path> mForwardStack;
//     std::filesystem::path mCurrentPath;
//     std::filesystem::path mRootPath;
//     std::shared_ptr<AssetNode> mCurrentAssetNode;

//   public:
//     FileExplorer(std::shared_ptr<ResourceManager> resourceManager, const std::filesystem::path &rootPath)
//         : mRootPath(rootPath), mCurrentPath(rootPath)
//     {
//     }
//     void Back()
//     {
//         if (!mBackStack.empty())
//         {
//             mForwardStack.push(mCurrentPath);
//             mCurrentPath = mBackStack.top();
//             mBackStack.pop();
//         }
//     }
//     void Forward()
//     {
//         if (!mForwardStack.empty())
//         {
//             mBackStack.push(mCurrentPath);
//             mCurrentPath = mForwardStack.top();
//             mForwardStack.pop();
//         }
//     }
// };
class EditorRenderSystem : public RenderSystem
{
  private:
    std::shared_ptr<IWindow> mWindow;
    std::shared_ptr<SamplerManager> mSamplerManager;
    std::shared_ptr<IRepository<PBRMaterial>> mPBRMaterialRepository;
    std::shared_ptr<IRepository<Texture2D>> mTexture2DRepository;
    // std::shared_ptr<ResourceManager> mResourceManager;

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

    std::unordered_map<UUID, VkDescriptorSet> mDescriptorSetMap;

  private:
    // Assets View
    std::shared_ptr<entt::registry> mAssetRegistry;
    std::filesystem::path mCurrentPath = mProjectPath;
    std::unordered_map<std::filesystem::path, entt::entity> mAssetMap;

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
    entt::entity mSelectedEntity = entt::null;
    entt::entity mHoveredEntity = entt::null;

  private:
    void HandleSwapchainOutOfDate() override;
    void InitialEditorRenderTargetImageLayout();

  private:
    void InitialFileExplore();
    void LoadResource(entt::entity parent);
    AssetType GetAssetTypeFromExtension(const std::filesystem::path &path);
    void AddAssetNode(entt::entity parent, const std::filesystem::path &path);
    void RemoveAssetNode(entt::entity entity);

    void SetDefaultWindowLayout();
    void RightClickMenu();
    void DockingSpace();
    void HierarchyWindow();
    void InspectorWindow();
    void ToolbarWindow();
    void SceneViewWindow();
    void AssetWindow();
    void FileExplore();
    void LoadUIIcon(const std::filesystem::path &iconPath, vk::DescriptorSet &descriptorSet);
    void CreateSceneView();

  private:
    void ShowTexture(const std::string &name, UUID textureID, ImVec2 size = ImVec2(50, 50));

  public:
    EditorRenderSystem(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                       std::shared_ptr<IConfigure> configure, std::shared_ptr<entt::registry> registry,
                       std::shared_ptr<RenderPassManager> renderPassManager,
                       std::shared_ptr<PipelineLayoutManager> pipelineLayoutManager,
                       std::shared_ptr<PipelineManager> pipelineManager,
                       std::shared_ptr<CommandBufferManager> commandBufferManager,
                       std::shared_ptr<SyncPrimitiveManager> syncPrimitiveManager,
                       std::shared_ptr<DescriptorManager> descriptorManager,
                       std::shared_ptr<SamplerManager> samplerManager, std::shared_ptr<BufferFactory> bufferFactory,
                       std::shared_ptr<ImageFactory> imageFactory, std::shared_ptr<IWindow> window,
                       std::shared_ptr<IRepository<PBRMaterial>> pbrMaterialRepository,
                       std::shared_ptr<IRepository<Texture2D>> texture2DRepository);
    ~EditorRenderSystem();
    virtual void Init() override;
    virtual void Tick(float deltaTime) override;
    virtual void Shutdown() override;
};

} // namespace MEngine