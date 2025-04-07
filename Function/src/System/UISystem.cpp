#include "System/UISystem.hpp"
#include "imgui.h"
#include <cstdint>
#include <vulkan/vulkan_structs.hpp>

namespace MEngine
{
UISystem::UISystem(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context, std::shared_ptr<IWindow> window,
                   std::shared_ptr<RenderPassManager> renderPassManager, std::shared_ptr<ImageManager> mImageManager)
    : mLogger(logger), mContext(context), mWindow(window), mRenderPassManager(renderPassManager),
      mImageManager(mImageManager)
{
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

    CreateSceneImage();
    CreateSceneImageView();
    CreateSceneRenderPass();
    CreateSceneFrameBuffer();
    CreateSceneDescriptorSetLayout();
    CreateSceneDescriptorSet();
    mLogger->Info("Uploading Fonts");
}
void UISystem::ProcessEvent(const SDL_Event *event)
{
    ImGui_ImplSDL3_ProcessEvent(static_cast<const SDL_Event *>(event));
}
void UISystem::CreateSceneRenderPass()
{
    // 1. 创建附件
    std::array<vk::AttachmentDescription, 1> attachments{
        vk::AttachmentDescription()
            .setFormat(mContext->GetSurfaceInfo().format.format) // Swapchain格式
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eLoad)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
            .setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal)};
    // 2. 创建子通道
    std::array<vk::SubpassDescription, 1> subpasses;
    std::array<vk::AttachmentReference, 1> colorRefs = {
        vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal) // Swapchain
    };
    subpasses[0]
        .setColorAttachments(colorRefs)                          // 颜色附件引用
        .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics); // 图形管线绑定点
    // 4. 创建渲染通道
    vk::RenderPassCreateInfo renderPassCreateInfo;
    renderPassCreateInfo
        .setAttachments(attachments) // 附件描述
        .setSubpasses(subpasses);    // 子通道描述
    auto renderPass = mContext->GetDevice().createRenderPassUnique(renderPassCreateInfo);
    if (!renderPass)
    {
        mLogger->Error("Failed to create UI-offline render pass");
    }
    mSceneRenderPasses = std::move(renderPass);
    mLogger->Debug("UI-offline render pass created successfully");
}
void UISystem::CreateSceneFrameBuffer()
{
    auto imageCount = mContext->GetSurfaceInfo().imageCount;
    for (int i = 0; i < imageCount; i++)
    {
        vk::ImageView attachments[] = {mImageSceneViews[i].get()};
        vk::FramebufferCreateInfo framebufferInfo;
        framebufferInfo.setRenderPass(mSceneRenderPasses.get())
            .setAttachments(attachments)
            .setWidth(mSceneWidth)
            .setHeight(mSceneHeight)
            .setLayers(1);
        auto framebuffer = mContext->GetDevice().createFramebufferUnique(framebufferInfo);
        if (!framebuffer)
        {
            mLogger->Error("Failed to create UI-offline framebuffer");
        }
        mSceneFramebuffers.push_back(std::move(framebuffer));
    }
    mLogger->Debug("UI-offline framebuffer created successfully");
}
void UISystem::CreateSceneImage()
{
    auto imageCount = mContext->GetSurfaceInfo().imageCount;
    for (int i = 0; i < imageCount; i++)
    {
        auto sceneImage = mImageManager->CreateUniqueTexture2D(vk::Extent2D(mSceneWidth, mSceneHeight),
                                                               mContext->GetSurfaceInfo().format.format, 1, nullptr);
        mSceneImages.push_back(std::move(sceneImage));
    }
}
void UISystem::CreateSceneImageView()
{
    auto imageCount = mContext->GetSurfaceInfo().imageCount;
    for (int i = 0; i < imageCount; i++)
    {
        auto sceneImageView =
            mImageManager->CreateImageView(mSceneImages[i]->GetImage(), mContext->GetSurfaceInfo().format.format);
        mImageSceneViews.push_back(std::move(sceneImageView));
    }
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
void UISystem::UpdateSceneDescriptorSet()
{
    auto imageCount = mContext->GetSurfaceInfo().imageCount;
    for (int i = 0; i < imageCount; i++)
    {
        vk::DescriptorImageInfo imageInfo;
        imageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
            .setImageView(mImageSceneViews[i].get())
            .setSampler(mSceneSampler.get());
        vk::WriteDescriptorSet writeSet;
        writeSet.setDstSet(mSceneDescriptorSets[i].get())
            .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
            .setImageInfo(imageInfo)
            .setDstBinding(0)
            .setDescriptorCount(1);
        mContext->GetDevice().updateDescriptorSets(writeSet, {});
    }
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
        ImGuiID left = ImGui::DockBuilderSplitNode(center, ImGuiDir_Left, 0.2f, nullptr, &center);
        ImGuiID right = ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.25f, nullptr, &center);
        ImGuiID bottom = ImGui::DockBuilderSplitNode(center, ImGuiDir_Down, 0.3f, nullptr, &center);
        // 设置初始尺寸
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
    if (size.x != mSceneWidth || size.y != mSceneHeight)
    {
        mSceneWidth = static_cast<uint32_t>(size.x);
        mSceneHeight = static_cast<uint32_t>(size.y);
        CreateSceneImage();
        CreateSceneImageView();
        CreateSceneFrameBuffer();
    }
    UpdateSceneDescriptorSet();
    ImGui::Text("SceneView Size: %.1f x %.1f", size.x, size.y);
    ImTextureID textureId =
        reinterpret_cast<ImTextureID>(static_cast<VkDescriptorSet>(mSceneDescriptorSets[mCurrentFrame].get()));
    ImGui::Image(textureId, ImVec2(size.x, size.y), ImVec2(0, 1), ImVec2(1, 0));
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