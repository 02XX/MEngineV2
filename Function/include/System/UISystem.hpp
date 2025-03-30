#pragma once
#include "Context.hpp"
#include "System.hpp"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_vulkan.h"
#include <cstdint>

namespace MEngine
{
class UISystem : public System
{
  public:
    UISystem(SDL_Window *window, vk::RenderPass renderPass, uint32_t subpass, vk::CommandBuffer commandBuffer);
    ~UISystem() override;
    void Init() override;
    void Tick(float deltaTime) override;
    void Shutdown() override;

  private:
    SDL_Window *mWindow;
    vk::RenderPass mRenderPass;
    uint32_t mSubpass;
    vk::CommandBuffer mCommandBuffer;
    ImGuiIO *mIO = nullptr;
};
} // namespace MEngine