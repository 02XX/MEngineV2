#include "System/UISystem.hpp"
#include "imgui.h"

namespace MEngine
{
UISystem::UISystem(SDL_Window *window, vk::RenderPass renderPass, uint32_t subpass, vk::CommandBuffer commandBuffer)
    : mWindow(window), mRenderPass(renderPass), mSubpass(subpass), mCommandBuffer(commandBuffer)
{
}
void UISystem::Init()
{
    auto &context = Context::Instance();
    // Initialize ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    mIO = &ImGui::GetIO();
    mIO->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

    // Initialize ImGui Vulkan Backend
    ImGui_ImplSDL3_InitForVulkan(mWindow);
    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.ApiVersion = context.GetInstanceVersion();
    initInfo.Instance = context.GetInstance();
    initInfo.PhysicalDevice = context.GetPhysicalDevice();
    initInfo.Device = context.GetDevice();
    initInfo.QueueFamily = context.GetQueueFamilyIndicates().graphicsFamily.value();
    // initInfo.DescriptorPool;
    initInfo.RenderPass = mRenderPass;
    initInfo.MinImageCount = context.GetSurfaceInfo().imageCount;
    initInfo.ImageCount = context.GetSurfaceInfo().imageCount;
    initInfo.MSAASamples = static_cast<VkSampleCountFlagBits>(vk::SampleCountFlagBits::e1);
    initInfo.DescriptorPoolSize = 1000;
    // optional
    initInfo.Subpass = mSubpass;
    ImGui_ImplVulkan_Init(&initInfo);
    mIsInit = true;
    LogD("UI System Initialized");
}
void UISystem::Tick(float deltaTime)
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::ShowDemoWindow();
    ImGui::Render();
    ImDrawData *drawData = ImGui::GetDrawData();
    bool isMinimized = (drawData->DisplaySize.x <= 0.0f || drawData->DisplaySize.y <= 0.0f);
    if (!isMinimized)
    {
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), mCommandBuffer);
    }
}
UISystem::~UISystem()
{
    if (!mIsShutdown)
        Shutdown();
}
void UISystem::Shutdown()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    mIsShutdown = true;
    LogD("UI System Destroyed");
}
} // namespace MEngine