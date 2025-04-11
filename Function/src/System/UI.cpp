#include "System/UI.hpp"
#include <vulkan/vulkan.hpp>

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
    mImageTransitionCommandBuffer = mCommandBufferManager->CreatePrimaryCommandBuffer(CommandBufferType::Graphic);
    mAssetSampler = mSamplerManager->CreateUniqueSampler(vk::Filter::eLinear, vk::Filter::eLinear);
    mSceneSampler = mSamplerManager->CreateUniqueSampler(vk::Filter::eLinear, vk::Filter::eLinear);
    mImageTransitionFence = mSyncPrimitiveManager->CreateFence();
    //  Initialize ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    mIO = &ImGui::GetIO();
    mIO->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();
    // Descriptor Pool
    CreateDescriptorPool();
    // Initialize ImGui Vulkan Backend
    ImGui_ImplSDL3_InitForVulkan(static_cast<SDL_Window *>(mWindow->GetNativeHandle()));
    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.ApiVersion = mContext->GetInstanceVersion();
    initInfo.Instance = mContext->GetInstance();
    initInfo.PhysicalDevice = mContext->GetPhysicalDevice();
    initInfo.Device = mContext->GetDevice();
    initInfo.QueueFamily = mContext->GetQueueFamilyIndicates().graphicsFamily.value();
    initInfo.Queue = mContext->GetGraphicQueue();
    initInfo.DescriptorPool = mUIDescriptorPool.get();
    initInfo.RenderPass = mRenderPassManager->GetRenderPass(RenderPassType::UI);
    initInfo.MinImageCount = mContext->GetSurfaceInfo().imageCount;
    initInfo.ImageCount = mContext->GetSurfaceInfo().imageCount;
    initInfo.MSAASamples = static_cast<VkSampleCountFlagBits>(vk::SampleCountFlagBits::e1);
    // optional
    initInfo.Subpass = 0;
    // initInfo.DescriptorPoolSize = 1000;
    ImGui_ImplVulkan_Init(&initInfo);

    // Upload Fonts

    // ImGuizmo
    ImGuizmo::SetImGuiContext(ImGui::GetCurrentContext());
    ImGuizmo::Enable(true);

    CreateSceneDescriptorSetLayout();
    CreateSceneDescriptorSet();
    LoadAsset();
    // mLogger->Info("Uploading Fonts");
}

void UI::ProcessEvent(const SDL_Event *event)
{
    ImGui_ImplSDL3_ProcessEvent(static_cast<const SDL_Event *>(event));
}

void UI::CreateDescriptorPool()
{
    // DescriptorPool
    vk::DescriptorPoolCreateInfo poolInfo;
    vk::DescriptorPoolSize poolSize(vk::DescriptorType::eCombinedImageSampler, 1000);
    poolInfo.setMaxSets(1000).setPoolSizes(poolSize).setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
    mUIDescriptorPool = mContext->GetDevice().createDescriptorPoolUnique(poolInfo);
    if (!mUIDescriptorPool)
    {
        mLogger->Error("Failed to create UI descriptor pool");
    }
}
void UI::CreateSceneDescriptorSetLayout()
{
    vk::DescriptorSetLayoutBinding binding;
    binding.setBinding(0)
        .setDescriptorCount(1)
        .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
        .setStageFlags(vk::ShaderStageFlagBits::eFragment);
    vk::DescriptorSetLayoutCreateInfo layoutInfo;
    layoutInfo.setBindings(binding);
    auto layout = mContext->GetDevice().createDescriptorSetLayoutUnique(layoutInfo);
    if (!layout)
    {
        mLogger->Error("Failed to create UI descriptor set layout");
    }
    mSceneDescriptorSetLayout = std::move(layout);
}
void UI::CreateSceneDescriptorSet()
{
    auto imageCount = mContext->GetSurfaceInfo().imageCount;
    for (int i = 0; i < imageCount; i++)
    {
        vk::DescriptorSetAllocateInfo allocInfo;
        allocInfo.setDescriptorPool(mUIDescriptorPool.get())
            .setSetLayouts(mSceneDescriptorSetLayout.get())
            .setDescriptorSetCount(1);
        auto descriptorSets = mContext->GetDevice().allocateDescriptorSetsUnique(allocInfo);
        if (descriptorSets.empty())
        {
            mLogger->Error("Failed to allocate UI descriptor set");
        }
        mSceneDescriptorSets.push_back(std::move(descriptorSets[0]));
    }
}
void UI::UpdateSceneDescriptorSet(vk::ImageView imageView, uint32_t imageIndex)
{
    mCurrentFrame = imageIndex;
    vk::DescriptorImageInfo imageInfo;
    imageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
        .setImageView(imageView)
        .setSampler(mSceneSampler.get());
    vk::WriteDescriptorSet writeSet;
    writeSet.setDstSet(mSceneDescriptorSets[imageIndex].get())
        .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
        .setImageInfo(imageInfo)
        .setDstBinding(0)
        .setDescriptorCount(1);
    mContext->GetDevice().updateDescriptorSets(writeSet, {});
}
void UI::LoadAsset()
{
    mImageTransitionCommandBuffer->reset();
    vk::SubmitInfo folderSubmitInfo;
    // 文件夹图标
    int folderThumbnailWidth, folderThumbnailHeight, folderThumbnailChannels;
    auto foldPath = mAssetsPath / "folder.png";
    stbi_set_flip_vertically_on_load(true);
    auto folderThumbnail = stbi_load(foldPath.string().c_str(), &folderThumbnailWidth, &folderThumbnailHeight,
                                     &folderThumbnailChannels, STBI_rgb_alpha);
    folderThumbnailChannels = 4;
    if (folderThumbnail)
    {
        // 1. 创建Image
        vk::DeviceSize folderSize = folderThumbnailWidth * folderThumbnailHeight * folderThumbnailChannels;
        mFolderImage = mImageFactory->CreateImage(ImageType::Texture2D,
                                                  vk::Extent3D(folderThumbnailWidth, folderThumbnailHeight, 1),
                                                  folderSize, folderThumbnail);
        // 2. 创建ImageView
        mFolderImageView = mImageFactory->CreateImageView(mFolderImage.get());
        // 3. 转换布局
        vk::ImageMemoryBarrier folderImagebarrier;
        folderImagebarrier.setImage(mFolderImage->GetHandle())
            .setOldLayout(vk::ImageLayout::eUndefined)
            .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
            .setSrcQueueFamilyIndex(mContext->GetQueueFamilyIndicates().graphicsFamily.value())
            .setDstQueueFamilyIndex(mContext->GetQueueFamilyIndicates().graphicsFamily.value())
            .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
            .setSrcAccessMask(vk::AccessFlagBits::eNoneKHR)
            .setDstAccessMask(vk::AccessFlagBits::eShaderRead);
        mImageTransitionCommandBuffer->begin(vk::CommandBufferBeginInfo());
        mImageTransitionCommandBuffer->pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe,
                                                       vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {},
                                                       folderImagebarrier);
        mImageTransitionCommandBuffer->end();
        folderSubmitInfo.setCommandBuffers(mImageTransitionCommandBuffer.get());

        //  4. 创建描述符集
        mFolderTexture =
            ImGui_ImplVulkan_AddTexture(mAssetSampler.get(), mFolderImageView.get(),
                                        static_cast<VkImageLayout>(vk::ImageLayout::eShaderReadOnlyOptimal));
        mLogger->Debug("Folder thumbnail loaded successfully");
    }
    else
    {
        mLogger->Error("Failed to load file thumbnail");
    }
    stbi_image_free(folderThumbnail);
    mContext->GetDevice().resetFences({mImageTransitionFence.get()});
    mContext->GetGraphicQueue().submit({folderSubmitInfo}, mImageTransitionFence.get());
    auto result = mContext->GetDevice().waitForFences({mImageTransitionFence.get()}, VK_TRUE, 1000000000); // 1s
    if (result != vk::Result::eSuccess)
    {
        mLogger->Error("Failed to wait for folder image transition fence");
    }

    mImageTransitionCommandBuffer->reset();
    vk::SubmitInfo fileSubmitInfo;
    // 文件图标
    int fileThumbnailWidth, fileThumbnailHeight, fileThumbnailChannels;
    auto filePath = mAssetsPath / "file.png";
    auto fileThumbnail = stbi_load(filePath.string().c_str(), &fileThumbnailWidth, &fileThumbnailHeight,
                                   &fileThumbnailChannels, STBI_rgb_alpha);
    fileThumbnailChannels = 4;
    if (fileThumbnail)
    {
        // 1. 创建Image
        vk::DeviceSize fileSize = fileThumbnailWidth * fileThumbnailHeight * fileThumbnailChannels;
        mFileImage = mImageFactory->CreateImage(
            ImageType::Texture2D, vk::Extent3D(fileThumbnailWidth, fileThumbnailHeight, 1), fileSize, fileThumbnail);
        // 2. 创建ImageView
        mFileImageView = mImageFactory->CreateImageView(mFileImage.get());
        // 3. 转换布局
        vk::ImageMemoryBarrier fileImagebarrier;
        fileImagebarrier.setImage(mFileImage->GetHandle())
            .setOldLayout(vk::ImageLayout::eUndefined)
            .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
            .setSrcQueueFamilyIndex(mContext->GetQueueFamilyIndicates().graphicsFamily.value())
            .setDstQueueFamilyIndex(mContext->GetQueueFamilyIndicates().graphicsFamily.value())
            .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
            .setSrcAccessMask(vk::AccessFlagBits::eNoneKHR)
            .setDstAccessMask(vk::AccessFlagBits::eShaderRead);
        mImageTransitionCommandBuffer->begin(vk::CommandBufferBeginInfo());
        mImageTransitionCommandBuffer->pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe,
                                                       vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {},
                                                       fileImagebarrier);
        mImageTransitionCommandBuffer->end();
        fileSubmitInfo.setCommandBuffers(mImageTransitionCommandBuffer.get());
        // 4. 创建描述符集
        mFileTexture = ImGui_ImplVulkan_AddTexture(mAssetSampler.get(), mFileImageView.get(),
                                                   static_cast<VkImageLayout>(vk::ImageLayout::eShaderReadOnlyOptimal));
        mLogger->Debug("File thumbnail loaded successfully");
    }
    else
    {
        mLogger->Error("Failed to load file thumbnail");
    }
    stbi_image_free(fileThumbnail);
    mContext->GetDevice().resetFences({mImageTransitionFence.get()});
    mContext->GetGraphicQueue().submit({fileSubmitInfo}, mImageTransitionFence.get());
    result = mContext->GetDevice().waitForFences({mImageTransitionFence.get()}, VK_TRUE, 1000000000); // 1s
    if (result != vk::Result::eSuccess)
    {
        mLogger->Error("Failed to wait for file image transition fence");
    }
}
void UI::DockingSpace()
{
    // 获取主视口尺寸
    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGuiID dockSpaceID = ImGui::DockSpaceOverViewport();
    if (mFirstRun)
    {
        ImGui::DockBuilderRemoveNode(dockSpaceID);
        ImGui::DockBuilderAddNode(dockSpaceID, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockSpaceID, viewport->WorkSize);

        // 1. 主区域拆分为底部（30%）和顶部（70%）
        ImGuiID dockBottomID, dockTopID;
        ImGui::DockBuilderSplitNode(dockSpaceID, ImGuiDir_Down, 0.3, &dockBottomID, &dockTopID);

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

        ImGui::DockBuilderFinish(dockSpaceID);
        mFirstRun = false;
    }
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
    mIsSceneViewPortChange = false;
    // 获取Camera
    auto &camera = mRegistry->get<CameraComponent>(mCameraEntity);
    camera.aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    if (width != mSceneWidth || height != mSceneHeight)
    {
        mSceneWidth = width;
        mSceneHeight = height;
        mIsSceneViewPortChange = true;
    }
    // imGuizmo
    const float gizmoLength = 64.0f;                  // Gizmo 视觉大小
    const ImVec2 gizmoSize(gizmoLength, gizmoLength); // 显示区域
    ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowSize.x, windowSize.y);

    glm::mat4 view = glm::transpose(camera.viewMatrix);
    glm::mat4 proj = glm::transpose(camera.projectionMatrix);
    float viewMatrix[16], projMatrix[16];
    memcpy(viewMatrix, glm::value_ptr(view), sizeof(float) * 16);
    memcpy(projMatrix, glm::value_ptr(proj), sizeof(float) * 16);
    float outputMatrix[16] = {0};
    // ImGuizmo::ViewManipulate(viewMatrix, projMatrix, ImGuizmo::ROTATE, ImGuizmo::LOCAL, outputMatrix, gizmoLength,
    //                          ImVec2(windowPos.x + windowSize.x - gizmoLength, windowPos.y), gizmoSize, 0x10101010);

    ImGui::Text("SceneView Size: %d x %d", width, height);
    ImGui::SetCursorPos(ImVec2(windowSize.x - 100, 20));
    ImGui::TextColored(ImVec4(1, 1, 0, 1), "FPS: %1.f", ImGui::GetIO().Framerate);
    // ImGui::TextColored(ImVec4(1, 1, 0, 1), "FPS: %1.f", 1.0f / mDeltaTime);
    ImTextureID textureId =
        reinterpret_cast<ImTextureID>(static_cast<VkDescriptorSet>(mSceneDescriptorSets[mCurrentFrame].get()));
    ImGui::Image(textureId, ImVec2(mSceneWidth, mSceneHeight), ImVec2(0, 1), ImVec2(1, 0));
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
            if (mCurrentAssetDir != std::filesystem::current_path())
            {
                mCurrentAssetDir = mCurrentAssetDir.parent_path();
            }
        }

        ImGui::SameLine();
        ImGui::Text("%s", mCurrentAssetDir.string().c_str());
        ImGui::EndMenuBar();
    }

    ImGui::Separator();

    // 2. 检查目录是否存在
    if (!std::filesystem::exists(mCurrentAssetDir))
    {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Directory not exists!");
        ImGui::End();
        return;
    }

    // 3. 设置列数
    const int columns = std::max(1, static_cast<int>(ImGui::GetContentRegionAvail().x / mThumbnailSize));
    ImGui::Columns(columns, "AssetColumns", false); // false = 不显示边框
    try
    {
        for (const auto &entry : std::filesystem::directory_iterator(mCurrentAssetDir))
        {
            if (!entry.is_directory())
                continue;

            ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<VkDescriptorSet>(mFolderTexture)),
                         ImVec2(mThumbnailSize, mThumbnailSize), ImVec2(0, 1), ImVec2(1, 0));
            ImGui::TextWrapped("%s", entry.path().filename().string().c_str());

            // 双击进入文件夹
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
            {
                mCurrentAssetDir = entry.path();
            }

            // 移动到下一列
            ImGui::NextColumn();
        }
        for (const auto &entry : std::filesystem::directory_iterator(mCurrentAssetDir))
        {
            if (entry.is_directory())
                continue;

            ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<VkDescriptorSet>(mFileTexture)),
                         ImVec2(mThumbnailSize, mThumbnailSize), ImVec2(0, 1), ImVec2(1, 0));
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
    ImGui_ImplSDL3_NewFrame();
    ImGui_ImplVulkan_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();
    DockingSpace();
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