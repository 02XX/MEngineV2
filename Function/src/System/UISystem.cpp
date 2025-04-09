#include "System/UISystem.hpp"
#include "entt/entity/fwd.hpp"

namespace MEngine
{
UISystem::UISystem(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context, std::shared_ptr<IWindow> window,
                   std::shared_ptr<entt::registry> registry, std::shared_ptr<RenderPassManager> renderPassManager,
                   std::shared_ptr<ImageManager> imageManager)
    : mLogger(logger), mContext(context), mWindow(window), mRenderPassManager(renderPassManager),
      mImageManager(imageManager), mRegistry(registry)
{
    if (!imageManager)
    {
        mImageManager = std::make_shared<ImageManager>(mLogger, mContext);
    }
    mSamplerManager = std::make_shared<SamplerManager>(mLogger, mContext);
}

void UISystem::Init()
{
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
    CreateSampler();
    LoadAsset();
    mLogger->Info("Uploading Fonts");
}
void UISystem::ProcessEvent(const SDL_Event *event)
{
    ImGui_ImplSDL3_ProcessEvent(static_cast<const SDL_Event *>(event));
}

void UISystem::CreateDescriptorPool()
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
void UISystem::CreateSceneDescriptorSetLayout()
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
void UISystem::CreateSampler()
{
    vk::SamplerCreateInfo samplerInfo;
    samplerInfo.setMagFilter(vk::Filter::eLinear)
        .setMinFilter(vk::Filter::eLinear)
        .setAddressModeU(vk::SamplerAddressMode::eRepeat)
        .setAddressModeV(vk::SamplerAddressMode::eRepeat)
        .setAddressModeW(vk::SamplerAddressMode::eRepeat)
        .setMipLodBias(0.0f)
        .setMinLod(0.0f)
        .setMaxLod(0.0f)
        .setBorderColor(vk::BorderColor::eIntOpaqueBlack);
    auto sampler = mContext->GetDevice().createSamplerUnique(samplerInfo);
    if (!sampler)
    {
        mLogger->Error("Failed to create UI sampler");
    }
    mSceneSampler = std::move(sampler);
}
void UISystem::CreateSceneDescriptorSet()
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
void UISystem::UpdateSceneDescriptorSet(vk::ImageView imageView, uint32_t imageIndex)
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
void UISystem::LoadAsset()
{
    // 3. 创建采样器
    mSampler = mSamplerManager->CreateUniqueSampler(vk::Filter::eLinear, vk::Filter::eLinear);
    // 文件夹图标
    int folderThumbnailWidth, folderThumbnailHeight, folderThumbnailChannels;
    auto foldPath = mAssetsPath / "folder.png";
    stbi_set_flip_vertically_on_load(true);
    auto folderThumbnail =
        stbi_load(foldPath.c_str(), &folderThumbnailWidth, &folderThumbnailHeight, &folderThumbnailChannels, 0);
    if (folderThumbnail)
    {
        // 1. 创建Image
        mFolderImage = mImageManager->CreateUniqueTexture2D(vk::Extent2D(folderThumbnailWidth, folderThumbnailHeight),
                                                            vk::Format::eR8G8B8A8Srgb, 1, folderThumbnail);
        // 2. 创建ImageView
        mFolderImageView = mImageManager->CreateImageView(mFolderImage->GetImage(), vk::Format::eR8G8B8A8Srgb);

        // 4. 创建描述符集
        mFolderTexture =
            ImGui_ImplVulkan_AddTexture(mSampler.get(), mFolderImageView.get(),
                                        static_cast<VkImageLayout>(vk::ImageLayout::eShaderReadOnlyOptimal));
        mLogger->Debug("Folder thumbnail loaded successfully");
    }
    else
    {
        mLogger->Error("Failed to load file thumbnail");
    }
    stbi_image_free(folderThumbnail);
    // 文件图标
    int fileThumbnailWidth, fileThumbnailHeight, fileThumbnailChannels;
    auto filePath = mAssetsPath / "file.png";
    auto fileThumbnail =
        stbi_load(filePath.c_str(), &fileThumbnailWidth, &fileThumbnailHeight, &fileThumbnailChannels, 0);
    if (fileThumbnail)
    {
        // 1. 创建Image
        mFileImage = mImageManager->CreateUniqueTexture2D(vk::Extent2D(fileThumbnailWidth, fileThumbnailHeight),
                                                          vk::Format::eR8G8B8A8Srgb, 1, fileThumbnail);
        // 2. 创建ImageView
        mFileImageView = mImageManager->CreateImageView(mFileImage->GetImage(), vk::Format::eR8G8B8A8Srgb);
        // 4. 创建描述符集
        mFileTexture = ImGui_ImplVulkan_AddTexture(mSampler.get(), mFileImageView.get(),
                                                   static_cast<VkImageLayout>(vk::ImageLayout::eShaderReadOnlyOptimal));
        mLogger->Debug("File thumbnail loaded successfully");
    }
    else
    {
        mLogger->Error("Failed to load file thumbnail");
    }
    stbi_image_free(fileThumbnail);
}
void UISystem::DockingSpace()
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
void UISystem::HierarchyWindow()
{
    ImGui::Begin("Hierarchy", nullptr, ImGuiWindowFlags_None);

    ImGui::End();
}
void UISystem::InspectorWindow()
{
    ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_None);
    ImGui::Text("Inspector");
    ImGui::End();
}
void UISystem::SceneViewWindow()
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
    float outputMatrix[16] = {0};              // 声明一个临时矩阵
    ImGuizmo::ViewManipulate(viewMatrix,       // view
                             projMatrix,       // projection
                             ImGuizmo::ROTATE, // operation
                             ImGuizmo::LOCAL,  // mode
                             outputMatrix,     // 输出矩阵（可为 null）
                             gizmoLength,      // length
                             ImVec2(windowPos.x + windowSize.x - gizmoLength, windowPos.y), // position
                             gizmoSize,                                                     // size
                             0x10101010                                                     // background color
    );

    ImGui::Text("SceneView Size: %d x %d", width, height);
    ImTextureID textureId =
        reinterpret_cast<ImTextureID>(static_cast<VkDescriptorSet>(mSceneDescriptorSets[mCurrentFrame].get()));
    ImGui::Image(textureId, ImVec2(mSceneWidth, mSceneHeight), ImVec2(0, 1), ImVec2(1, 0));
    ImGui::End();
}
void UISystem::AssetWindow()
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
void UISystem::Tick(float deltaTime)
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
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), mCommandBuffer);
    }
}
void UISystem::Shutdown()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    mLogger->Info("UI System Shutdown");
}
} // namespace MEngine