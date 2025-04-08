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
    ImVec2 size = ImGui::GetContentRegionAvail();
    // 尺寸变化检测
    uint32_t width = static_cast<uint32_t>(size.x);
    uint32_t height = static_cast<uint32_t>(size.y);
    mIsSceneViewPortChange = false;
    if (width != mSceneWidth || height != mSceneHeight)
    {
        mSceneWidth = width;
        mSceneHeight = height;
        mIsSceneViewPortChange = true;
    }
    ImGui::Text("SceneView Size: %d x %d", width, height);
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