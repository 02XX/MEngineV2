#include "System/EditorRenderSystem.hpp"
#include "imgui.h"

namespace MEngine
{
// void OnEntityCreate(entt::registry &registry, entt::entity entity)
// {
//     auto &assetComponent = registry.get<AssetsComponent>(entity);
//     registry.sort<AssetsComponent>(
//         [&](const AssetsComponent &lhs, const AssetsComponent &rhs) { return lhs.type < rhs.type; });
// }
EditorRenderSystem::EditorRenderSystem(
    std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context, std::shared_ptr<IConfigure> configure,
    std::shared_ptr<entt::registry> registry, std::shared_ptr<RenderPassManager> renderPassManager,
    std::shared_ptr<PipelineLayoutManager> pipelineLayoutManager, std::shared_ptr<PipelineManager> pipelineManager,
    std::shared_ptr<CommandBufferManager> commandBufferManager,
    std::shared_ptr<SyncPrimitiveManager> syncPrimitiveManager, std::shared_ptr<DescriptorManager> descriptorManager,
    std::shared_ptr<SamplerManager> samplerManager, std::shared_ptr<BufferFactory> bufferFactory,
    std::shared_ptr<ImageFactory> imageFactory, std::shared_ptr<IWindow> window,
    std::shared_ptr<IRepository<PBRMaterial>> pbrMaterialRepository,
    std::shared_ptr<IRepository<Texture2D>> texture2DRepository)
    : RenderSystem(logger, context, configure, registry, renderPassManager, pipelineLayoutManager, pipelineManager,
                   commandBufferManager, syncPrimitiveManager, descriptorManager, bufferFactory, imageFactory),
      mWindow(window), mSamplerManager(samplerManager), mPBRMaterialRepository(pbrMaterialRepository),
      mTexture2DRepository(texture2DRepository)
{
}
void EditorRenderSystem::Init()
{
    RenderSystem::Init();
    InitialEditorRenderTargetImageLayout();
    mIconTransitionCommandBuffer = mCommandBufferManager->CreatePrimaryCommandBuffer(CommandBufferType::Graphic);
    mIconSampler = mSamplerManager->CreateUniqueSampler(vk::Filter::eLinear, vk::Filter::eLinear);
    mSceneSampler = mSamplerManager->CreateUniqueSampler(vk::Filter::eLinear, vk::Filter::eLinear);
    mIconTransitionFence = mSyncPrimitiveManager->CreateFence();
    //  Initialize ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    mIO = &ImGui::GetIO();
    mIO->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();
    // Initialize ImGui Vulkan Backend
    ImGui_ImplSDL3_InitForVulkan(static_cast<SDL_Window *>(mWindow->GetNativeHandle()));
    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.ApiVersion = mContext->GetInstanceVersion();
    initInfo.Instance = mContext->GetInstance();
    initInfo.PhysicalDevice = mContext->GetPhysicalDevice();
    initInfo.Device = mContext->GetDevice();
    initInfo.QueueFamily = mContext->GetQueueFamilyIndicates().graphicsFamily.value();
    initInfo.Queue = mContext->GetGraphicQueue();
    initInfo.RenderPass = mRenderPassManager->GetRenderPass(RenderPassType::EditorUI);
    initInfo.MinImageCount = mContext->GetSurfaceInfo().imageCount;
    initInfo.ImageCount = mContext->GetSurfaceInfo().imageCount;
    initInfo.MSAASamples = static_cast<VkSampleCountFlagBits>(vk::SampleCountFlagBits::e1);
    // optional
    initInfo.Subpass = 0;
    initInfo.DescriptorPoolSize = 1000;
    ImGui_ImplVulkan_Init(&initInfo);
    mWindow->SetEventCallback(
        [](const void *event) { ImGui_ImplSDL3_ProcessEvent(static_cast<const SDL_Event *>(event)); });

    // Upload Fonts
    mIO->Fonts->AddFontDefault();
    mMSYHFont = mIO->Fonts->AddFontFromFileTTF((mUIResourcePath / "Font" / "MSYH.TTC").string().c_str(), 16.0f, nullptr,
                                               mIO->Fonts->GetGlyphRangesChineseSimplifiedCommon());
    mIO->FontDefault = mMSYHFont;
    mAssetRegistry = std::make_shared<entt::registry>();
    // mAssetRegistry->on_construct<AssetsComponent>().connect<&OnEntityCreate>();
    // ImGuizmo
    ImGuizmo::SetImGuiContext(ImGui::GetCurrentContext());
    ImGuizmo::Enable(true);
    // SceneView
    CreateSceneView();

    // auto defaultTexture = mTextureManager->GetDefaultTexture();
    // mDefaultTextureDescriptorSet =
    //     ImGui_ImplVulkan_AddTexture(mSceneSampler.get(), defaultTexture->GetImageView(),
    //                                 static_cast<VkImageLayout>(vk::ImageLayout::eShaderReadOnlyOptimal));
    // 添加编辑相机
    auto editorCameraEntity = mRegistry->create();
    mRegistry->emplace<TransformComponent>(
        editorCameraEntity, TransformComponent(glm::vec3(0.0f, 0.0f, 5.0f), glm::quat(), glm::vec3(1.0f, 1.0f, 1.0f)));
    mRegistry->emplace<CameraComponent>(editorCameraEntity,
                                        CameraComponent{.isMainCamera = true, .isEditCamera = true});
    mRegistry->emplace<InputComponent>(editorCameraEntity);
    InitialFileExplore();
    mLogger->Info("EditorRenderSystem Initialized");
}
void EditorRenderSystem::InitialFileExplore()
{
    mCurrentPath = mProjectPath;
    // Icons
    LoadUIIcon(mUIResourcePath / "Icon" / "folder.png", mFolderIcon);
    LoadUIIcon(mUIResourcePath / "Icon" / "file.png", mFileIcon);
    // entity
    auto entity = mAssetRegistry->create();
    mAssetRegistry->emplace<AssetsComponent>(entity);
    auto &assetComponent = mAssetRegistry->get<AssetsComponent>(entity);
    assetComponent.path = mProjectPath;
    assetComponent.name = mProjectPath.filename().string();
    assetComponent.type = AssetType::Folder;
    assetComponent.iconDescriptorSet = mFolderIcon;
    assetComponent.parent = entt::null;
    mAssetMap[assetComponent.path] = entity;
    LoadResource(entity);
}
AssetType EditorRenderSystem::GetAssetTypeFromExtension(const std::filesystem::path &path)
{
    auto ext = path.extension().string();
    if (ext == ".pbrmat" || ext == ".phongmat")
        return AssetType::PBRMaterial;
    if (ext == ".tex2D" || ext == ".png" || ext == ".jpg")
        return AssetType::Texture2D;
    if (ext == ".glb" || ext == ".fbx")
        return AssetType::Model;
    if (ext == ".anim")
        return AssetType::Animation;
    if (ext == ".shader")
        return AssetType::Shader;
    if (ext == ".wav" || ext == ".mp3")
        return AssetType::Audio;
    return AssetType::File;
}
void EditorRenderSystem::LoadResource(entt::entity parent)
{
    auto &parentAssetComponent = mAssetRegistry->get<AssetsComponent>(parent);
    auto directory = std::filesystem::directory_iterator(parentAssetComponent.path);
    std::vector<std::filesystem::directory_entry> entries;
    for (auto &entry : std::filesystem::directory_iterator(parentAssetComponent.path))
    {
        entries.push_back(entry);
    }
    std::sort(entries.begin(), entries.end(), [](const auto &a, const auto &b) {
        if (a.is_directory() != b.is_directory())
            return a.is_directory();
        return a.path().filename().string() < b.path().filename().string();
    });
    for (auto entry : entries)
    {
        auto entity = mAssetRegistry->create();
        mAssetRegistry->emplace<AssetsComponent>(entity);
        auto &assetComponent = mAssetRegistry->get<AssetsComponent>(entity);
        assetComponent.path = entry.path();
        assetComponent.name = entry.path().filename().string();
        assetComponent.parent = parent;
        parentAssetComponent.children.push_back(entity);
        mAssetMap[assetComponent.path] = entity;
        if (entry.is_directory())
        {
            assetComponent.type = AssetType::Folder;
            assetComponent.iconDescriptorSet = mFolderIcon;
            LoadResource(entity);
        }
        else
        {
            assetComponent.type = GetAssetTypeFromExtension(entry.path());
            assetComponent.iconDescriptorSet = mFileIcon;
            switch (assetComponent.type)
            {
            case AssetType::PBRMaterial: {
                // // PBR
                // auto material = mPBRMaterialRepository->LoadFromFile(entry.path());
                break;
            }
            case AssetType::Texture2D: {
                // auto texture = mTexture2DRepository->LoadFromFile(entry.path());
                break;
            }
            default:
                mLogger->Error("Unknown asset type: {}", magic_enum::enum_name(assetComponent.type));
                break;
            }
        }
    }
}
void EditorRenderSystem::AddAssetNode(entt::entity parent, const std::filesystem::path &path)
{
}
void EditorRenderSystem::RemoveAssetNode(entt::entity entity)
{
}
void EditorRenderSystem::Tick(float deltaTime)
{
    Prepare();
    // TickRotationMatrix();
    CollectEntities(); // Collect same material render entities
    ImGui_ImplSDL3_NewFrame();
    ImGui_ImplVulkan_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();
    DockingSpace();
    if (mIsFirstFrame)
    {
        SetDefaultWindowLayout();
        mIsFirstFrame = false;
    }
    RightClickMenu();
    HierarchyWindow();
    InspectorWindow();
    AssetWindow();
    SceneViewWindow();
    ToolbarWindow();
    // RenderShadowDepthPass();  // Shadow pass
    // void RenderDeferred();
    RenderForward();
    // RenderSkyPass();          // Sky pass
    // RenderTranslucencyPass(); // Translucency pass
    // RenderPostProcessPass();  // Post process pass
    // RenderUIPass(deltaTime); // UI pass
    ImGui::Render();
    ImDrawData *drawData = ImGui::GetDrawData();
    bool isMinimized = (drawData->DisplaySize.x <= 0.0f || drawData->DisplaySize.y <= 0.0f);
    if (!isMinimized)
    {
        // 将Scene转为ShaderReadOnlyOptimal布局
        auto renderTargets = mRenderPassManager->GetRenderTargets();
        vk::ImageMemoryBarrier preBarrier;
        preBarrier.setImage(renderTargets[mFrameIndex].colorImage->GetHandle())
            .setOldLayout(vk::ImageLayout::eColorAttachmentOptimal)
            .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
            .setSrcQueueFamilyIndex(mContext->GetQueueFamilyIndicates().graphicsFamily.value())
            .setDstQueueFamilyIndex(mContext->GetQueueFamilyIndicates().graphicsFamily.value())
            .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
            .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
            .setDstAccessMask(vk::AccessFlagBits::eShaderRead);
        mGraphicCommandBuffers[mFrameIndex]->pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                                             vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {},
                                                             preBarrier);
        vk::ClearValue clearValue(std::array<float, 4>{0.1f, 0.1f, 0.1f, 1.0f});
        vk::RenderPassBeginInfo renderPassBeginInfo;
        auto editorFrameBuffers = mRenderPassManager->GetFrameBuffer(RenderPassType::EditorUI);
        auto renderPass = mRenderPassManager->GetRenderPass(RenderPassType::EditorUI);
        auto editorFrameResources = mRenderPassManager->GetEditorRenderTargets();
        renderPassBeginInfo.setRenderPass(renderPass)
            .setFramebuffer(editorFrameBuffers[mFrameIndex])
            .setRenderArea(vk::Rect2D({0, 0}, mContext->GetSurfaceInfo().extent))
            .setClearValues(clearValue);
        mGraphicCommandBuffers[mFrameIndex]->beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), mGraphicCommandBuffers[mFrameIndex].get());
        mGraphicCommandBuffers[mFrameIndex]->endRenderPass();
        // 将Scene转为ColorAttachmentOptimal布局
        vk::ImageMemoryBarrier postBarrier;
        postBarrier.setImage(renderTargets[mFrameIndex].colorImage->GetHandle())
            .setOldLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
            .setNewLayout(vk::ImageLayout::eColorAttachmentOptimal)
            .setSrcQueueFamilyIndex(mContext->GetQueueFamilyIndicates().graphicsFamily.value())
            .setDstQueueFamilyIndex(mContext->GetQueueFamilyIndicates().graphicsFamily.value())
            .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
            .setSrcAccessMask(vk::AccessFlagBits::eShaderRead)
            .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
        mGraphicCommandBuffers[mFrameIndex]->pipelineBarrier(vk::PipelineStageFlagBits::eFragmentShader,
                                                             vk::PipelineStageFlagBits::eColorAttachmentOutput, {}, {},
                                                             {}, postBarrier);
    }
    CopyColorAttachmentToSwapchainImage(*mRenderPassManager->GetEditorRenderTargets()[mFrameIndex].colorImage);
    Present();
}
void EditorRenderSystem::Shutdown()
{
    mIO->Fonts->Clear();
    mIO->Fonts->ClearInputData();
    mIO->Fonts->ClearTexData();
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    RenderSystem::Shutdown();
    mIsShutdown = true;
    mLogger->Info("EditorRenderSystem Shutdown");
}
void EditorRenderSystem::HandleSwapchainOutOfDate()
{
    RenderSystem::HandleSwapchainOutOfDate();
    auto extent = mContext->GetSurfaceInfo().extent;
    mRenderPassManager->RecreateEditorRenderTargetFrameBuffer(extent.width, extent.height);
    mSceneViewPortWidth = extent.width;
    mSceneViewPortHeight = extent.height;
    InitialEditorRenderTargetImageLayout();
    CreateSceneView();
}
void EditorRenderSystem::InitialEditorRenderTargetImageLayout()
{
    auto fence = mSyncPrimitiveManager->CreateFence();
    std::vector<vk::SubmitInfo> submitInfos;
    auto commandBuffer = mCommandBufferManager->CreatePrimaryCommandBuffer(CommandBufferType::Graphic);
    commandBuffer->reset();
    // RenderTarget ImageLayout
    {
        commandBuffer->begin(vk::CommandBufferBeginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
        auto renderTargetImages = mRenderPassManager->GetEditorRenderTargets();
        for (auto &renderTarget : renderTargetImages)
        {
            // Render Target ImageLayout
            vk::ImageMemoryBarrier editorUIImageMemoryBarrier;
            editorUIImageMemoryBarrier.setImage(renderTarget.colorImage->GetHandle())
                .setOldLayout(vk::ImageLayout::eUndefined)
                .setNewLayout(vk::ImageLayout::eColorAttachmentOptimal)
                .setSrcAccessMask(vk::AccessFlagBits::eNone)
                .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
                .setSubresourceRange(vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
            commandBuffer->pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe,
                                           vk::PipelineStageFlagBits::eColorAttachmentOutput, {}, {}, {},
                                           editorUIImageMemoryBarrier);
        }
        commandBuffer->end();
        vk::SubmitInfo submitInfo;
        submitInfo.setCommandBuffers({commandBuffer.get()});
        submitInfos.push_back(submitInfo);
    }
    // 提交命令缓冲区
    mContext->SubmitToGraphicQueue(submitInfos, fence.get());
    auto result = mContext->GetDevice().waitForFences({fence.get()}, VK_TRUE, 1000000000); // 1s
    if (result != vk::Result::eSuccess)
    {
        mLogger->Error("Failed to transition editor render target image layout");
        throw std::runtime_error("Failed to transition editor render target image layout");
    }
    mLogger->Info("Editor rendertarget image layout transition completed");
}
void EditorRenderSystem::CreateSceneView()
{
    for (auto sceneDescriptorSet : mSceneDescriptorSets)
    {
        ImGui_ImplVulkan_RemoveTexture(sceneDescriptorSet);
    }
    mSceneDescriptorSets.clear();
    auto editorRenderTargets = mRenderPassManager->GetRenderTargets();
    for (size_t i = 0; i < editorRenderTargets.size(); ++i)
    {
        mSceneDescriptorSets.push_back(
            ImGui_ImplVulkan_AddTexture(mSceneSampler.get(), editorRenderTargets[i].colorImageView.get(),
                                        static_cast<VkImageLayout>(vk::ImageLayout::eShaderReadOnlyOptimal)));
    }
    mLogger->Info("Scene View Descriptor Set Created");
}
void EditorRenderSystem::LoadUIIcon(const std::filesystem::path &iconPath, vk::DescriptorSet &descriptorSet)
{
    mIconTransitionCommandBuffer->reset();
    int iconWidth, iconHeight, iconChannels;
    stbi_set_flip_vertically_on_load(true);
    auto iconData = stbi_load(iconPath.string().c_str(), &iconWidth, &iconHeight, &iconChannels, STBI_rgb_alpha);
    iconChannels = 4;
    if (iconData)
    {
        // 1. 创建Image
        vk::DeviceSize folderSize = iconWidth * iconHeight * iconChannels;
        mIconImages.push_back(mImageFactory->CreateImage(ImageType::Texture2D, vk::Extent3D(iconWidth, iconHeight, 1),
                                                         folderSize, iconData));
        // 2. 创建ImageView
        auto imageView = mImageFactory->CreateImageView(mIconImages.back().get());
        mIconImageViews.push_back(std::move(imageView));
        // 3. 转换布局
        vk::ImageMemoryBarrier imagebarrier;
        imagebarrier.setImage(mIconImages.back()->GetHandle())
            .setOldLayout(vk::ImageLayout::eUndefined)
            .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
            .setSrcQueueFamilyIndex(mContext->GetQueueFamilyIndicates().graphicsFamily.value())
            .setDstQueueFamilyIndex(mContext->GetQueueFamilyIndicates().graphicsFamily.value())
            .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
            .setSrcAccessMask(vk::AccessFlagBits::eNoneKHR)
            .setDstAccessMask(vk::AccessFlagBits::eShaderRead);
        mIconTransitionCommandBuffer->begin(vk::CommandBufferBeginInfo());
        mIconTransitionCommandBuffer->pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe,
                                                      vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {},
                                                      imagebarrier);
        mIconTransitionCommandBuffer->end();
        vk::SubmitInfo submitInfo;
        submitInfo.setCommandBuffers(mIconTransitionCommandBuffer.get());
        //  4. 创建描述符集
        descriptorSet =
            ImGui_ImplVulkan_AddTexture(mIconSampler.get(), mIconImageViews.back().get(),
                                        static_cast<VkImageLayout>(vk::ImageLayout::eShaderReadOnlyOptimal));
        mContext->GetDevice().resetFences({mIconTransitionFence.get()});
        mContext->SubmitToGraphicQueue({submitInfo}, mIconTransitionFence.get());
        auto result = mContext->GetDevice().waitForFences({mIconTransitionFence.get()}, VK_TRUE, 1000000000); // 1s
        if (result != vk::Result::eSuccess)
        {
            mLogger->Error("Failed to create icon: {}", iconPath.string());
        }
        mLogger->Debug("loaded icon: {}", iconPath.string());
    }
    else
    {
        mLogger->Error("Failed to load icon: {}", iconPath.string());
    }
    stbi_image_free(iconData);
}
void EditorRenderSystem::SetDefaultWindowLayout()
{
    ImGui::DockBuilderRemoveNode(mDockSpaceID);
    ImGui::DockBuilderAddNode(mDockSpaceID, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(mDockSpaceID, mMainViewport->WorkSize);
    // 1. 主区域拆分为底部（30%）和顶部（70%）
    ImGuiID dockBottomID, dockTopID;
    ImGui::DockBuilderSplitNode(mDockSpaceID, ImGuiDir_Down, 0.3, &dockBottomID, &dockTopID);
    // 2. 顶部区域拆分为左（30%）和剩余部分（70%）
    ImGuiID dockLeftID, remainingTop;
    ImGui::DockBuilderSplitNode(dockTopID, ImGuiDir_Left, 0.3, &dockLeftID, &remainingTop);
    // 3. 剩余部分（70%）拆分为中（60%）和右（40%）
    ImGuiID dockCenterID, dockRightID;
    ImGui::DockBuilderSplitNode(remainingTop, ImGuiDir_Right, 0.4, &dockRightID, &dockCenterID);
    // 4. 中部拆分上(20%)和下(80%)
    ImGuiID dockTopCenterID, dockBottomCenterID;
    ImGui::DockBuilderSplitNode(dockCenterID, ImGuiDir_Up, 0.15, &dockTopCenterID, &dockBottomCenterID);
    // 绑定窗口
    ImGui::DockBuilderDockWindow("SceneView", dockBottomCenterID); // 中间
    ImGui::DockBuilderDockWindow("Hierarchy", dockLeftID);         // 左侧
    ImGui::DockBuilderDockWindow("Inspector", dockRightID);        // 右侧
    ImGui::DockBuilderDockWindow("Assets", dockBottomID);          // 底部
    ImGui::DockBuilderDockWindow("Toolbar", dockTopCenterID);      // 顶部
    ImGui::DockBuilderFinish(mDockSpaceID);
}
void EditorRenderSystem::RightClickMenu()
{
}
void EditorRenderSystem::DockingSpace()
{
    mMainViewport = ImGui::GetMainViewport();
    mDockSpaceID = ImGui::DockSpaceOverViewport();
}

void EditorRenderSystem::HierarchyWindow()
{
    ImGui::Begin("Hierarchy", nullptr, ImGuiWindowFlags_None);
    if (ImGui::IsWindowFocused())
    {
        mAssetsHoveredEntity = entt::null;
        mAssetsSelectedEntity = entt::null;
    }
    // 显示实体列表
    auto entities = mRegistry->view<TransformComponent>();
    for (auto entity : entities)
    {
        if (mRegistry->all_of<CameraComponent>(entity) && mRegistry->get<CameraComponent>(entity).isEditCamera)
        {
            continue;
        }
        ImGui::PushID(static_cast<int>(entt::to_integral(entity)));
        auto entityLabel = std::format("Entity {}", entt::to_integral(entity));
        if (ImGui::Selectable(entityLabel.c_str(), mSelectedEntity == entity))
        {
            mSelectedEntity = entity;
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup))
        {
            mHoveredEntity = entity;
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
            {
                mSelectedEntity = entity;
            }
        }
        ImGui::PopID();
    }
    if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered())
    {
        mHoveredEntity = entt::null;
    }
    // 右键菜单
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ImGui::IsWindowHovered())
    {
        ImGui::OpenPopup("HierarchyContextMenu");
    }
    if (ImGui::BeginPopupContextWindow("HierarchyContextMenu", ImGuiPopupFlags_MouseButtonRight))
    {
        if (ImGui::MenuItem("Create Entity"))
        {
            auto entity = mRegistry->create();
            mRegistry->emplace<TransformComponent>(entity);
            mSelectedEntity = entity;
        }
        if (mHoveredEntity != entt::null)
        {
            ImGui::Separator();
            if (ImGui::MenuItem("Delete Entity"))
            {
                mContext->GetDevice().waitIdle();
                mRegistry->destroy(mSelectedEntity);
                mSelectedEntity = entt::null;
                mHoveredEntity = entt::null;
            }
        }
        ImGui::EndPopup();
    }
    ImGui::End();
}
void EditorRenderSystem::ShowTexture(const std::string &name, UUID textureID, ImVec2 size)
{
    auto texture2D = mTexture2DRepository->Get(textureID);
    ImGui::Columns(2, nullptr, false); // 创建两列布局，不显示分隔线
    ImGui::SetColumnWidth(0, 100);     // 固定文本列宽
    ImGui::Text("%s", name.c_str());
    ImGui::NextColumn(); // 切换到图片列
    VkDescriptorSet id;
    if (mDescriptorSetMap.find(textureID) == mDescriptorSetMap.end())
    {
        id = ImGui_ImplVulkan_AddTexture(texture2D->GetSampler(), texture2D->GetImageView(),
                                         static_cast<VkImageLayout>(vk::ImageLayout::eShaderReadOnlyOptimal));
        mDescriptorSetMap[textureID] = id;
    }
    else
    {
        id = mDescriptorSetMap[textureID];
    }
    ImGui::Image(reinterpret_cast<ImTextureID>(id), size, ImVec2(0, 1), ImVec2(1, 0));
    ImGui::Dummy(ImVec2(0, 5));

    ImGui::Columns(1); // 恢复单列布局
}
void EditorRenderSystem::InspectorMaterial(MaterialComponent &materialComponent)
{
    if (ImGui::CollapsingHeader("Material Attribute", ImGuiTreeNodeFlags_DefaultOpen))
    {
        auto &material = materialComponent.material;
        ImGui::Text("RenderType: %s", magic_enum::enum_name(material->GetRenderType()).data());
        switch (material->GetRenderType())
        {
        case RenderType::ForwardOpaquePBR:
        case RenderType::ForwardTransparentPBR: {
            auto pbrMaterial = static_cast<PBRMaterial *>(material);
            auto pbrMaterialParams = pbrMaterial->GetMaterialParams();
            auto albedoColor = glm::value_ptr(pbrMaterialParams.parameters.albedo);
            if (ImGui::ColorEdit3("Albedo", albedoColor))
            {
                pbrMaterialParams.parameters.albedo = glm::vec3(albedoColor[0], albedoColor[1], albedoColor[2]);
                pbrMaterial->SetMaterialParams(pbrMaterialParams);
                mContext->GetDevice().waitIdle();
                mPBRMaterialRepository->Update(pbrMaterial->GetID(), pbrMaterial);
                // mPBRMaterialRepository->SaveToFile()
            }
            ImGui::Dummy(ImVec2(0, 5));
            if (ImGui::BeginCombo("##albedo",
                                  mTexture2DRepository->Get(pbrMaterial->GetAlbedoMapID())->GetName().c_str()))
            {
                for (const auto &texture : mTexture2DRepository->GetAll())
                {
                    ImGui::PushID(texture->GetID().ToString().c_str());
                    if (ImGui::Selectable(texture->GetName().c_str(),
                                          texture->GetID() == pbrMaterial->GetAlbedoMapID()))
                    {
                        pbrMaterial->SetAlbedoMapID(texture->GetID());
                        mContext->GetDevice().waitIdle();
                        mPBRMaterialRepository->Update(pbrMaterial->GetID(), pbrMaterial);
                    }
                    ImGui::PopID();
                }
                ImGui::EndCombo();
            }
            ShowTexture("Albedo Map", pbrMaterial->GetAlbedoMapID());
            if (ImGui::BeginCombo("##normal",
                                  mTexture2DRepository->Get(pbrMaterial->GetNormalMapID())->GetName().c_str()))
            {
                for (const auto &texture : mTexture2DRepository->GetAll())
                {
                    ImGui::PushID(texture->GetID().ToString().c_str());
                    if (ImGui::Selectable(texture->GetName().c_str(),
                                          texture->GetID() == pbrMaterial->GetNormalMapID()))
                    {
                        pbrMaterial->SetNormalMapID(texture->GetID());
                        mContext->GetDevice().waitIdle();
                        mPBRMaterialRepository->Update(pbrMaterial->GetID(), pbrMaterial);
                    }
                    ImGui::PopID();
                }
                ImGui::EndCombo();
            }
            ShowTexture("Normal Map", pbrMaterial->GetNormalMapID());
            if (ImGui::BeginCombo(
                    "##roughness",
                    mTexture2DRepository->Get(pbrMaterial->GetMetallicRoughnessMapID())->GetName().c_str()))
            {
                for (const auto &texture : mTexture2DRepository->GetAll())
                {
                    ImGui::PushID(texture->GetID().ToString().c_str());
                    if (ImGui::Selectable(texture->GetName().c_str(),
                                          texture->GetID() == pbrMaterial->GetMetallicRoughnessMapID()))
                    {
                        pbrMaterial->SetMetallicRoughnessMapID(texture->GetID());
                        mContext->GetDevice().waitIdle();
                        mPBRMaterialRepository->Update(pbrMaterial->GetID(), pbrMaterial);
                    }
                    ImGui::PopID();
                }
                ImGui::EndCombo();
            }
            ShowTexture("ARM Map", pbrMaterial->GetMetallicRoughnessMapID());
            ShowTexture("Emissive Map", pbrMaterial->GetEmissiveMapID());
            break;
        }
        case RenderType::ForwardOpaquePhong:
        case RenderType::ForwardTransparentPhong:
        case RenderType::Deferred:
            break;
        };
    }
}
void EditorRenderSystem::InspectorWindow()
{
    ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_None);
    // 1. 显示选中实体的属性
    if (mSelectedEntity != entt::null)
    {
        auto entityLabel = std::format("Entity {}", static_cast<uint32_t>(mSelectedEntity));
        ImGui::Text("%s", entityLabel.c_str());
        // 显示TransomComponent
        if (mRegistry->any_of<TransformComponent>(mSelectedEntity))
        {
            if (ImGui::CollapsingHeader("Transform Component", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::BeginChild("Transform", ImVec2(0, 100), true);
                auto &transform = mRegistry->get<TransformComponent>(mSelectedEntity);
                auto modelMatrix = transform.modelMatrix;
                float matrixTranslation[3], matrixRotation[3], matrixScale[3];
                ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(modelMatrix), matrixTranslation, matrixRotation,
                                                      matrixScale);
                ImGui::DragFloat3("Position", matrixTranslation, 0.1f);
                ImGui::DragFloat3("Rotation", matrixRotation, 0.1f);
                ImGui::DragFloat3("Scale", matrixScale, 0.1f);
                ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale,
                                                        glm::value_ptr(modelMatrix));
                // 分解矩阵
                glm::vec3 translation, scale, skew;
                glm::quat rotation;
                glm::vec4 perspective;
                if (glm::decompose(modelMatrix, scale, rotation, translation, skew, perspective))
                {
                    transform.position = translation;
                    transform.rotation = rotation;
                    transform.scale = scale;
                }
                ImGui::EndChild();
            }
        }
        // 显示MeshComponent
        if (mRegistry->any_of<MeshComponent>(mSelectedEntity))
        {
        }
        // 显示MaterialComponent
        if (mRegistry->any_of<MaterialComponent>(mSelectedEntity))
        {
            auto &material = mRegistry->get<MaterialComponent>(mSelectedEntity);
            if (ImGui::CollapsingHeader("Material Component", ImGuiTreeNodeFlags_DefaultOpen))
            {
                // 获取所有材质
                auto materials = mPBRMaterialRepository->GetAll();
                std::string name = material.material->GetName() + "@" + material.material->GetID().ToString();
                // 显示材质列表
                if (ImGui::BeginCombo("Material", name.c_str()))
                {
                    for (int i = 0; i < materials.size(); i++)
                    {
                        ImGui::PushID(i);
                        std::string materialName = materials[i]->GetName() + "@" + materials[i]->GetID().ToString();
                        bool isSelected = (materials[i]->GetID() == material.material->GetID());
                        if (ImGui::Selectable(materialName.c_str(), isSelected))
                        {
                            material.material = materials[i];
                        }
                        ImGui::PopID();
                    }
                    ImGui::EndCombo();
                }
            }
            InspectorMaterial(material);
        }
    }
    // 2. 显示选中资源的属性
    if (mAssetsSelectedEntity != entt::null)
    {
        auto entityLabel = std::format("Asset {}", static_cast<uint32_t>(mAssetsSelectedEntity));
        ImGui::Text("%s", entityLabel.c_str());
        // 显示AssetsComponent
        if (mAssetRegistry->any_of<AssetsComponent>(mAssetsSelectedEntity))
        {
            if (ImGui::CollapsingHeader("Assets Component", ImGuiTreeNodeFlags_DefaultOpen))
            {
                auto &asset = mAssetRegistry->get<AssetsComponent>(mAssetsSelectedEntity);
                ImGui::Text("Path: %s", asset.path.string().c_str());
                ImGui::Text("Name: %s", asset.name.c_str());
                ImGui::Text("Type: %s", magic_enum::enum_name(asset.type).data());
            }
        }
        // 显示MaterialComponent
        if (mAssetRegistry->any_of<MaterialComponent>(mAssetsSelectedEntity))
        {
            auto &material = mAssetRegistry->get<MaterialComponent>(mAssetsSelectedEntity);
            InspectorMaterial(material);
        }
    }
    ImGui::End();
}
void EditorRenderSystem::ToolbarWindow()
{
    ImGui::Begin("Toolbar", nullptr, ImGuiWindowFlags_None);
    // 显示工具栏按钮
    ImGui::BeginGroup();
    {
        if (ImGui::Button("Play"))
        {
            mIsPlay = !mIsPlay;
        }
        ImGui::SameLine();
        if (ImGui::Button("Pause"))
        {
            mIsPause = !mIsPause;
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop"))
        {
            mIsStop = !mIsStop;
        }
    }
    ImGui::EndGroup();
    ImGui::SameLine();
    // 显示窗口标题 FPS 等信息
    ImGui::BeginGroup();
    {
        auto sceneWindow = ImGui::FindWindowByName("SceneView");
        ImGui::Text("SceneView Size: %1.f x %1.f", sceneWindow->ContentSize.x, sceneWindow->ContentSize.y);
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "FPS: %1.f", ImGui::GetIO().Framerate);
        if (ImGui::RadioButton("Translate", mGuizmoOperation == ImGuizmo::TRANSLATE) || ImGui::IsKeyDown(ImGuiKey_W))
            mGuizmoOperation = ImGuizmo::TRANSLATE;
        ImGui::SameLine();
        if (ImGui::RadioButton("Rotate", mGuizmoOperation == ImGuizmo::ROTATE) || ImGui::IsKeyDown(ImGuiKey_E))
            mGuizmoOperation = ImGuizmo::ROTATE;
        ImGui::SameLine();
        if (ImGui::RadioButton("Scale", mGuizmoOperation == ImGuizmo::SCALE) || ImGui::IsKeyDown(ImGuiKey_R))
            mGuizmoOperation = ImGuizmo::SCALE;
        if (ImGui::RadioButton("Local", mGuizmoMode == ImGuizmo::LOCAL))
            mGuizmoMode = ImGuizmo::LOCAL;
        ImGui::SameLine();
        if (ImGui::RadioButton("World", mGuizmoMode == ImGuizmo::WORLD))
            mGuizmoMode = ImGuizmo::WORLD;
    }
    ImGui::EndGroup();
    ImGui::End();
}
void EditorRenderSystem::SceneViewWindow()
{
    ImGui::Begin("SceneView", nullptr, ImGuiWindowFlags_None);
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetContentRegionAvail();
    // 尺寸变化检测
    uint32_t width = static_cast<uint32_t>(windowSize.x);
    uint32_t height = static_cast<uint32_t>(windowSize.y);
    mIsSceneViewPortChanged = false;
    // 获取Camera
    auto &cameraComponent = mRegistry->get<CameraComponent>(mMainCameraEntity);
    cameraComponent.aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    if (width != mSceneViewPortWidth || height != mSceneViewPortHeight)
    {
        mSceneViewPortWidth = width;
        mSceneViewPortHeight = height;
        if (mSceneViewPortWidth != 0 && mSceneViewPortHeight != 0)
        {
            mContext->GetDevice().waitIdle();
            mRenderPassManager->RecreateRenderTargetFrameBuffer(width, height);
            InitialRenderTargetImageLayout();
            CreateSceneView();
        }
    }
    else
    {
        // 显示场景视图
        if (!mSceneDescriptorSets.empty())
        {
            ImTextureID textureId =
                reinterpret_cast<ImTextureID>(static_cast<VkDescriptorSet>(mSceneDescriptorSets[mFrameIndex]));
            ImGui::Image(textureId, ImVec2(mSceneViewPortWidth, mSceneViewPortHeight), ImVec2(0, 1), ImVec2(1, 0));
        }
        // 3. imGuizmo
        ImVec2 imagePos = ImGui::GetItemRectMin();
        ImVec2 imageSize = ImGui::GetItemRectSize();
        ImGuizmo::SetRect(imagePos.x, imagePos.y, imageSize.x, imageSize.y);
        ImGuizmo::SetDrawlist(ImGui::GetCurrentWindow()->DrawList);
        if (mSelectedEntity != entt::null)
        {
            auto &transform = mRegistry->get<TransformComponent>(mSelectedEntity);
            auto modelMatrix = transform.modelMatrix;
            ImGuizmo::Manipulate(glm::value_ptr(cameraComponent.viewMatrix),
                                 glm::value_ptr(cameraComponent.projectionMatrix), mGuizmoOperation, mGuizmoMode,
                                 glm::value_ptr(modelMatrix));
            if (ImGuizmo::IsUsing())
            {
                // 分解矩阵
                glm::vec3 translation, scale, skew;
                glm::quat rotation;
                glm::vec4 perspective;
                if (glm::decompose(modelMatrix, scale, rotation, translation, skew, perspective))
                {
                    transform.position = translation;
                    transform.rotation = rotation;
                    transform.scale = scale;
                }
            }
        }
    }
    ImGui::End();
}
void EditorRenderSystem::AssetWindow()
{
    ImGui::Begin("Assets", nullptr, ImGuiWindowFlags_MenuBar);
    if (ImGui::IsWindowFocused())
    {
        mHoveredEntity = entt::null;
        mSelectedEntity = entt::null;
    }
    // 1. 菜单栏
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::Button("<-"))
        {
            if (mCurrentPath != mProjectPath)
            {
                mCurrentPath = mCurrentPath.parent_path();
            }
        }
        ImGui::SameLine();
        ImGui::Text("%s", mCurrentPath.string().c_str());
        ImGui::EndMenuBar();
    }
    ImGui::Separator();
    FileExplore();
    // 4. 右键菜单
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ImGui::IsWindowHovered())
    {
        ImGui::OpenPopup("AssetContextMenu");
    }
    if (ImGui::BeginPopupContextWindow("AssetContextMenu", ImGuiPopupFlags_MouseButtonRight))
    {
        if (ImGui::MenuItem("Create Folder"))
        {
            std::string folderName = "NewFolder";
            std::filesystem::path newFolderPath = mCurrentPath / folderName;
            int count = 0;
            while (std::filesystem::exists(newFolderPath))
            {
                folderName = std::format("NewFolder{}", ++count);
                newFolderPath = mCurrentPath / folderName;
            }
            std::filesystem::create_directory(newFolderPath);
        }
        if (ImGui::MenuItem("Create Material"))
        {
            std::string materialName = "NewMaterial.mat";
            int count = 0;
            std::filesystem::path newMaterialPath = mCurrentPath / materialName;
            while (std::filesystem::exists(newMaterialPath))
            {
                materialName = std::format("NewMaterial{}.mat", ++count);
                newMaterialPath = mCurrentPath / materialName;
            }
            // mMaterialManager->CreateMaterial(newMaterialPath);
        }
        if (ImGui::MenuItem("Delete"))
        {
        }
        ImGui::EndPopup();
    }
    ImGui::End();
}
void EditorRenderSystem::FileExplore()
{
    if (!std::filesystem::exists(mCurrentPath))
    {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Directory not exists!");
        ImGui::End();
        return;
    }
    else
    {
        const int itemWidth = mIconSize + 10;
        const int itemHeight = mIconSize + 20;
        int columns = std::max(1, static_cast<int>(ImGui::GetContentRegionAvail().x / itemWidth));
        if (mAssetMap.find(mCurrentPath) != mAssetMap.end())
        {
            ImGui::Columns(columns, "AssetColumns", false); // false = 不显示边框
            auto currentNode = mAssetMap[mCurrentPath];
            auto &assetComponent = mAssetRegistry->get<AssetsComponent>(currentNode);
            for (auto &entity : assetComponent.children)
            {
                auto &asset = mAssetRegistry->get<AssetsComponent>(entity);
                ImGui::PushID(static_cast<int>(entity));
                // 绘制图标按钮
                const bool isSelected = (mAssetsSelectedEntity == entity);
                if (ImGui::Selectable("##btn", isSelected,
                                      ImGuiSelectableFlags_AllowItemOverlap | ImGuiSelectableFlags_AllowDoubleClick,
                                      ImVec2(itemWidth, itemHeight)))
                {
                    mAssetsSelectedEntity = entity;
                    // 处理双击
                    if (ImGui::IsMouseDoubleClicked(0))
                    {
                        if (asset.type == AssetType::Folder)
                        {
                            mCurrentPath = mCurrentPath / asset.name;
                        }
                    }
                }
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup))
                {
                    mAssetsHoveredEntity = entity;
                    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
                        mAssetsSelectedEntity = entity;
                    if (ImGui::IsMouseDragging(0))
                    {
                        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
                        {
                            // 设置拖拽数据
                            ImGui::SetDragDropPayload("ASSET_ITEM", &entity, sizeof(entity));
                            // 显示拖拽预览（图标）
                            ImGui::Image(
                                reinterpret_cast<ImTextureID>(static_cast<VkDescriptorSet>(asset.iconDescriptorSet)),
                                ImVec2(mIconSize, mIconSize), ImVec2(0, 1), ImVec2(1, 0));
                            // 可选：添加文字说明
                            ImGui::Text("拖动 %s", asset.name.c_str());
                            ImGui::EndDragDropSource();
                        }
                    }
                }
                ImVec2 ContainerMinPos = ImGui::GetItemRectMin();
                ImVec2 ContainerMaxPos = ImGui::GetItemRectMax();
                ImVec2 ContainerSize = ImGui::GetItemRectSize();
                // 绘制图标
                ImVec2 iconPos = ImVec2(ContainerMinPos.x, ContainerMinPos.y);
                ImGui::SetCursorScreenPos(iconPos);
                ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<VkDescriptorSet>(asset.iconDescriptorSet)),
                             ImVec2(mIconSize, mIconSize), ImVec2(0, 1), ImVec2(1, 0));
                // 绘制文本
                ImVec2 textPos =
                    ImVec2(ContainerMinPos.x + (ContainerSize.x - mIconSize) / 2, ContainerMinPos.y + mIconSize);
                ImGui::SetCursorScreenPos(textPos);
                ImGui::Text("%s", asset.name.c_str());

                ImGui::PopID();
                ImGui::NextColumn(); // 移动到下一列
            }
            ImGui::Columns(1); // 恢复单列模式
        }
        else
        {
            mLogger->Error("Asset not found in registry {}", mCurrentPath.string());
        }
    }
}

EditorRenderSystem::~EditorRenderSystem()
{
    if (!IsShutdown())
    {
        Shutdown();
    }
}
} // namespace MEngine