#pragma once
#include "Interface/ILogger.hpp"
#include "Interface/IWindow.hpp"
#include "NoCopyable.hpp"
#include "SDL3/SDL.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_vulkan.h"
#include <vulkan/vulkan.hpp>
namespace MEngine
{
struct WindowConfig
{
    int width;
    int height;
    std::string title;
};
class SDLWindow final : public IWindow, public NoCopyable
{
  private:
    std::shared_ptr<ILogger> mLogger;

  private:
    SDL_Window *mWindow = nullptr;
    int mWidth = 0;
    int mHeight = 0;
    SDL_Event mEvent;
    WindowConfig mConfig;

  public:
    SDLWindow(std::shared_ptr<ILogger> logger, WindowConfig config);
    ~SDLWindow();

  public:
    vk::SurfaceKHR GetSurface(vk::Instance instance) const override;
    std::vector<const char *> GetInstanceRequiredExtensions() const override;

  public:
    void PollEvents() override;
    bool ShouldClose() const override;

  public:
    int GetWidth() const override;
    int GetHeight() const override;
    void *GetNativeHandle() const override;
};
} // namespace MEngine