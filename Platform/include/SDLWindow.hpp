#pragma once
#include "Interface/IConfigure.hpp"
#include "Interface/ILogger.hpp"
#include "Interface/IWindow.hpp"
#include "NoCopyable.hpp"
#include "SDL3/SDL.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_vulkan.h"
#include "imgui_impl_sdl3.h"
#include <memory>
#include <queue>
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
    std::shared_ptr<IConfigure> mConfigure;

  private:
    SDL_Window *mWindow = nullptr;
    SDL_Event mEvent;
    WindowConfig mConfig;
    bool mShouldClose = false;
    EventCallback mEventCallback = nullptr;
    std::vector<EventCallback> mEventCallbacks;

  public:
    SDLWindow(std::shared_ptr<ILogger> logger, std::shared_ptr<IConfigure> configure);
    ~SDLWindow();

  public:
    vk::SurfaceKHR GetSurface(vk::Instance instance) const override;
    std::vector<const char *> GetInstanceRequiredExtensions() const override;

    /**
     * @brief Deprecated: Use Manual Control.
     *
     * @param enable
     */
    void SetVSync(bool enable) override;
    void SetEventCallback(EventCallback callback) override;

  public:
    void PollEvents() override;
    bool ShouldClose() const override;

  public:
    int GetWidth() const override;
    int GetHeight() const override;
    void *GetNativeHandle() const override;
};
} // namespace MEngine

namespace nlohmann
{
template <> struct adl_serializer<MEngine::WindowConfig>
{
    static void to_json(json &j, const MEngine::WindowConfig &w)
    {
        j = json{{"Width", w.width}, {"Height", w.height}, {"Title", w.title}};
    }

    static void from_json(const json &j, MEngine::WindowConfig &w)
    {
        w.width = j.value("Width", w.width);
        w.height = j.value("Height", w.height);
        w.title = j.value("Title", w.title);

        if (w.width <= 0)
        {
            throw std::runtime_error("Window width must be positive");
        }
        if (w.height <= 0)
        {
            throw std::runtime_error("Window height must be positive");
        }
        if (w.title.empty())
        {
            throw std::runtime_error("Window title cannot be empty");
        }
    }
};
} // namespace nlohmann