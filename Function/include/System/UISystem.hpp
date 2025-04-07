#pragma once

#include "Context.hpp"
#include "Interface/ILogger.hpp"
#include "Interface/IWindow.hpp"
#include "RenderPassManager.hpp"
#include "System/ISystem.hpp"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_vulkan.h"
#include <memory>
#include <vulkan/vulkan_handles.hpp>
namespace MEngine
{
class UISystem : public ISystem
{
  private:
    std::shared_ptr<ILogger> mLogger;
    std::shared_ptr<Context> mContext;
    std::shared_ptr<IWindow> mWindow;
    std::shared_ptr<RenderPassManager> mRenderPassManager;

  private:
    vk::UniqueDescriptorPool mUIDescriptorPool;
    ImGuiIO *mIO;
    vk::CommandBuffer mCommandBuffer;

  public:
    UISystem(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context, std::shared_ptr<IWindow> window,
             std::shared_ptr<RenderPassManager> renderPassManager);
    void SetCommandBuffer(vk::CommandBuffer commandBuffer)
    {
        mCommandBuffer = commandBuffer;
    }
    void HeirarchyWindow();
    void InspectorWindow();
    void AssetWindow();

    void ProcessEvent(const SDL_Event *event);
    void Init() override;
    void Tick(float deltaTime) override;
    void Shutdown() override;
};

} // namespace MEngine