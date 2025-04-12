#include "System/UI.hpp"

namespace MEngine
{
UI::UI(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context, std::shared_ptr<IWindow> window,
       std::shared_ptr<RenderPassManager> renderPassManager, std::shared_ptr<ImageFactory> imageFactory,
       std::shared_ptr<CommandBufferManager> commandBufferManager,
       std::shared_ptr<SyncPrimitiveManager> syncPrimitiveManager, std::shared_ptr<SamplerManager> samplerManager,
       std::shared_ptr<entt::registry> registry)
    : mLogger(logger), mContext(context), mWindow(window), mRenderPassManager(renderPassManager),
      mImageFactory(imageFactory), mCommandBufferManager(commandBufferManager),
      mSyncPrimitiveManager(syncPrimitiveManager), mSamplerManager(samplerManager), mRegistry(registry)
{
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
    initInfo.RenderPass = mRenderPassManager->GetRenderPass(RenderPassType::UI);
    initInfo.MinImageCount = mContext->GetSurfaceInfo().imageCount;
    initInfo.ImageCount = mContext->GetSurfaceInfo().imageCount;
    initInfo.MSAASamples = static_cast<VkSampleCountFlagBits>(vk::SampleCountFlagBits::e1);
    // optional
    initInfo.Subpass = 0;
    initInfo.DescriptorPoolSize = 1000;
    ImGui_ImplVulkan_Init(&initInfo);

    // Upload Fonts

    // ImGuizmo
    ImGuizmo::SetImGuiContext(ImGui::GetCurrentContext());
    ImGuizmo::Enable(true);

    // Icons
    LoadUIIcon(mUIResourcePath / "Icon" / "folder.png", mFolderIcon);
    LoadUIIcon(mUIResourcePath / "Icon" / "file.png", mFileIcon);
}
void UI::ProcessEvent(const SDL_Event *event)
{
    ImGui_ImplSDL3_ProcessEvent(static_cast<const SDL_Event *>(event));
}
void UI::SetSceneViewPort(const std::vector<vk::ImageView> &imageViews)
{
    mSceneImageViews = imageViews;
    for (size_t i = 0; i < mSceneImageViews.size(); ++i)
    {
        mSceneDescriptorSets.push_back(
            ImGui_ImplVulkan_AddTexture(mSceneSampler.get(), mSceneImageViews[i],
                                        static_cast<VkImageLayout>(vk::ImageLayout::eShaderReadOnlyOptimal)));
    }
}
void UI::CollectEntity()
{
    // camera
    auto entities = mRegistry->view<CameraComponent>();
    for (auto entity : entities)
    {
        auto &camera = entities.get<CameraComponent>(entity);
        if (camera.isMainCamera)
        {
            mMainCamera = entity;
            break;
        }
    }
}
void UI::LoadUIIcon(const std::filesystem::path &iconPath, vk::DescriptorSet &descriptorSet)
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
void UI::SetDefaultWindowLayout()
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
    // 绑定窗口
    ImGui::DockBuilderDockWindow("SceneView", dockCenterID); // 中间
    ImGui::DockBuilderDockWindow("Hierarchy", dockLeftID);   // 左侧
    ImGui::DockBuilderDockWindow("Inspector", dockRightID);  // 右侧
    ImGui::DockBuilderDockWindow("Assets", dockBottomID);    // 底部
    ImGui::DockBuilderFinish(mDockSpaceID);
}
void UI::DockingSpace()
{
    mMainViewport = ImGui::GetMainViewport();
    mDockSpaceID = ImGui::DockSpaceOverViewport();
}
void UI::HierarchyWindow()
{
    ImGui::Begin("Hierarchy", nullptr, ImGuiWindowFlags_None);

    ImGui::End();
}
void UI::InspectorWindow()
{
    ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_None);
    ImGui::Text("Inspector");
    ImGui::End();
}
void UI::SceneViewWindow()
{
    ImGui::Begin("SceneView", nullptr, ImGuiWindowFlags_None);
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetContentRegionAvail();
    // 尺寸变化检测
    uint32_t width = static_cast<uint32_t>(windowSize.x);
    uint32_t height = static_cast<uint32_t>(windowSize.y);
    mIsSceneViewPortChanged = false;
    // 获取Camera
    auto &camera = mRegistry->get<CameraComponent>(mMainCamera);
    camera.aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    if (width != mSceneViewPortWidth || height != mSceneViewPortHeight)
    {
        mSceneViewPortWidth = width;
        mSceneViewPortHeight = height;
        mIsSceneViewPortChanged = true;
    }
    // // imGuizmo
    // const float gizmoLength = 64.0f;                  // Gizmo 视觉大小
    // const ImVec2 gizmoSize(gizmoLength, gizmoLength); // 显示区域
    // ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowSize.x, windowSize.y);

    // glm::mat4 view = glm::transpose(camera.viewMatrix);
    // glm::mat4 proj = glm::transpose(camera.projectionMatrix);
    // float viewMatrix[16], projMatrix[16];
    // memcpy(viewMatrix, glm::value_ptr(view), sizeof(float) * 16);
    // memcpy(projMatrix, glm::value_ptr(proj), sizeof(float) * 16);
    // float outputMatrix[16] = {0};
    // // ImGuizmo::ViewManipulate(viewMatrix, projMatrix, ImGuizmo::ROTATE, ImGuizmo::LOCAL, outputMatrix, gizmoLength,
    // //                          ImVec2(windowPos.x + windowSize.x - gizmoLength, windowPos.y), gizmoSize,
    // 0x10101010);

    ImGui::Text("SceneView Size: %d x %d", width, height);
    ImGui::SetCursorPos(ImVec2(windowSize.x - 100, 20));
    ImGui::TextColored(ImVec4(1, 1, 0, 1), "FPS: %1.f", ImGui::GetIO().Framerate);
    // ImGui::TextColored(ImVec4(1, 1, 0, 1), "FPS: %1.f", 1.0f / mDeltaTime);
    ImTextureID textureId =
        reinterpret_cast<ImTextureID>(static_cast<VkDescriptorSet>(mSceneDescriptorSets[mImageIndex]));
    ImGui::Image(textureId, ImVec2(mSceneViewPortWidth, mSceneViewPortHeight), ImVec2(0, 1), ImVec2(1, 0));
    ImGui::End();
}
void UI::AssetWindow()
{
    ImGui::Begin("Assets", nullptr, ImGuiWindowFlags_MenuBar);
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
    // 2. 检查目录是否存在
    if (!std::filesystem::exists(mCurrentPath))
    {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Directory not exists!");
        ImGui::End();
        return;
    }
    // 3. 设置列数
    const int columns = std::max(1, static_cast<int>(ImGui::GetContentRegionAvail().x / mIconSize));
    ImGui::Columns(columns, "AssetColumns", false); // false = 不显示边框
    try
    {
        for (const auto &entry : std::filesystem::directory_iterator(mCurrentPath))
        {
            if (!entry.is_directory())
                continue;

            ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<VkDescriptorSet>(mFolderIcon)),
                         ImVec2(mIconSize, mIconSize), ImVec2(0, 1), ImVec2(1, 0));
            ImGui::TextWrapped("%s", entry.path().filename().string().c_str());

            // 双击进入文件夹
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
            {
                mCurrentPath = entry.path();
            }
            // 移动到下一列
            ImGui::NextColumn();
        }
        for (const auto &entry : std::filesystem::directory_iterator(mCurrentPath))
        {
            if (entry.is_directory())
                continue;

            ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<VkDescriptorSet>(mFileIcon)),
                         ImVec2(mIconSize, mIconSize), ImVec2(0, 1), ImVec2(1, 0));
            ImGui::TextWrapped("%s", entry.path().filename().string().c_str());

            // 双击文件
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
            {
            }
            // 移动到下一列
            ImGui::NextColumn();
        }
    }
    catch (const std::exception &e)
    {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error: %s", e.what());
    }
    ImGui::Columns(1);
    ImGui::End();
}
void UI::RecordUICommandBuffer(vk::CommandBuffer commandBuffer)
{
    CollectEntity();
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
    SceneViewWindow();
    HierarchyWindow();
    InspectorWindow();
    AssetWindow();
    ImGui::Render();
    ImDrawData *drawData = ImGui::GetDrawData();
    bool isMinimized = (drawData->DisplaySize.x <= 0.0f || drawData->DisplaySize.y <= 0.0f);
    if (!isMinimized)
    {
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    }
}
UI::~UI()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    mLogger->Info("UI System Shutdown");
}
} // namespace MEngine