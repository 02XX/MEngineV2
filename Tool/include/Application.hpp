#pragma once

#include "Context.hpp"
#include "IScene.hpp"
#include "Logger.hpp"
#include "MEngine.hpp"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_video.h"
#include "SDL3/SDL_vulkan.h"
#include "Scene/DefaultScene.hpp"
#include <memory>

namespace MEngine
{
class MENGINE_API Application final
{
  private:
    bool mIsRunning;
    SDL_Window *mWindow;
    // std::shared_ptr<IScene> mCurrScene;

  public:
    Application();
    ~Application();
    void Run();
};
} // namespace MEngine
