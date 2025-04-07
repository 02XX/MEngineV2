#include "System/UISystem.hpp"

namespace MEngine
{
UISystem::UISystem(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context, std::shared_ptr<IWindow> window,
                   std::shared_ptr<RenderPassManager> renderPassManager, std::shared_ptr<ImageManager> mImageManager)
    : mLogger(logger), mContext(context), mWindow(window), mRenderPassManager(renderPassManager),
      mImageManager(mImageManager)
{
    if (!mImageManager)
    {
        mImageManager = std::make_unique<ImageManager>(mLogger, mContext);
    }
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

    CreateSceneDescriptorSetLayout();
    CreateSceneDescriptorSet();
    CreateSampler();
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
void UISystem::DockingSpace()
{
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    // 获取主视口尺寸
    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    // 创建主窗口容器
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("MainDockSpaceWindow", nullptr, flags);
    ImGui::PopStyleVar(2);

    // 创建 DockSpace
    ImGuiID dockSpaceID = ImGui::GetID("MainDockSpace");
    ImGui::DockSpace(dockSpaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

    // 首次运行时初始化默认布局
    if (mFirstRun)
    {
        ImGui::DockBuilderRemoveNode(dockSpaceID); // 清除默认布局
        ImGui::DockBuilderAddNode(dockSpaceID, ImGuiDockNodeFlags_PassthruCentralNode);

        // 划分区域
        ImGuiID center = dockSpaceID; // 保留中心区域
        ImGuiID bottom = ImGui::DockBuilderSplitNode(center, ImGuiDir_Down, 0.3f, nullptr, &center);
        ImGuiID left = ImGui::DockBuilderSplitNode(center, ImGuiDir_Left, 0.2f, nullptr, &center);
        ImGuiID right = ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.25f, nullptr, &center);
        ImGui::DockBuilderSetNodeSize(center, ImVec2(mSceneWidth, mSceneHeight));
        // 分配窗口到停靠节点
        ImGui::DockBuilderDockWindow("SceneView", center);
        ImGui::DockBuilderDockWindow("Hierarchy", left);
        ImGui::DockBuilderDockWindow("Inspector", right);
        ImGui::DockBuilderDockWindow("Assets", bottom);

        ImGui::DockBuilderFinish(dockSpaceID);
        mFirstRun = false;
    }

    ImGui::End();
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
    ImVec2 size = ImGui::GetContentRegionAvail();

    // 四舍五入转换 + 防零处理
    uint32_t newWidth = static_cast<uint32_t>(size.x);
    uint32_t newHeight = static_cast<uint32_t>(size.y);
    newWidth = newWidth ? newWidth : 1;
    newHeight = newHeight ? newHeight : 1;
    // 尺寸变化检测
    if (newWidth != mSceneWidth || newHeight != mSceneHeight)
    {
        // mSceneWidth = newWidth;
        // mSceneHeight = newHeight;
        // mRenderPassManager->RecreateFrameBuffer(mSceneWidth, mSceneHeight);
    }
    mLogger->Debug("SceneView Size: {} x {}", mSceneWidth, mSceneHeight);
    ImGui::Text("SceneView Size: %d x %d", mSceneWidth, mSceneHeight);
    ImTextureID textureId =
        reinterpret_cast<ImTextureID>(static_cast<VkDescriptorSet>(mSceneDescriptorSets[mCurrentFrame].get()));
    ImGui::Image(textureId, ImVec2(mSceneWidth, mSceneHeight), ImVec2(0, 1), ImVec2(1, 0));
    ImGui::End();
}
void UISystem::AssetWindow()
{
    ImGui::Begin("Assets", nullptr, ImGuiWindowFlags_None);
    ImGui::Text("Assets");
    ImGui::End();
}
void UISystem::Tick(float deltaTime)
{
    ImGui_ImplSDL3_NewFrame();
    ImGui_ImplVulkan_NewFrame();
    ImGui::NewFrame();
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