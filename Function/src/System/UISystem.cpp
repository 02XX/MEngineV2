#include "System/UISystem.hpp"

namespace MEngine
{
UISystem::UISystem(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context, std::shared_ptr<IWindow> window,
                   std::shared_ptr<RenderPassManager> renderPassManager)
    : mLogger(logger), mContext(context), mWindow(window), mRenderPassManager(renderPassManager)
{
}
void UISystem::Init()
{
    //  Initialize ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    mIO = &ImGui::GetIO();
    mIO->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    ImGui::StyleColorsDark();
    // DescriptorPool
    vk::DescriptorPoolCreateInfo poolInfo;
    vk::DescriptorPoolSize poolSize(vk::DescriptorType::eCombinedImageSampler, 1000);
    poolInfo.setMaxSets(1000).setPoolSizes(poolSize).setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
    mUIDescriptorPool = mContext->GetDevice().createDescriptorPoolUnique(poolInfo);
    if (!mUIDescriptorPool)
    {
        mLogger->Error("Failed to create UI descriptor pool");
    }
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

    mLogger->Info("Uploading Fonts");
}
void UISystem::ProcessEvent(const SDL_Event *event)
{
    ImGui_ImplSDL3_ProcessEvent(static_cast<const SDL_Event *>(event));
}
void HeirarchyWindow()
{
    ImGui::Begin("Heirarchy", nullptr, ImGuiWindowFlags_None);

    ImGui::End();
}
void InspectorWindow()
{
    ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_None);
    ImGui::Text("Inspector");
    ImGui::End();
}
void AssetWindow()
{
    ImGui::Begin("Asset", nullptr, ImGuiWindowFlags_None);
    ImGui::Text("Asset");
    ImGui::End();
}
void UISystem::Tick(float deltaTime)
{
    ImGui_ImplSDL3_NewFrame();
    ImGui_ImplVulkan_NewFrame();
    ImGui::NewFrame();
    HeirarchyWindow();
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