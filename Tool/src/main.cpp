#include "Application.hpp"
#include "Logger.hpp"
using namespace MEngine;
int main()
{
    // Application app;
    // app.Run();

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        LogE("Failed to initialize SDL: {}", SDL_GetError());
    }
    auto mWindow = SDL_CreateWindow("MEngine", 800, 600, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
    if (!mWindow)
    {
        LogE("Failed to create window: {}", SDL_GetError());
        throw std::runtime_error("Failed to create window");
    }
    Uint32 extensionCount = 0;
    auto extensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);
    std::vector<const char *> requiredExtensions(extensions, extensions + extensionCount);
    // vulkan
    auto &context = Context::Instance();
    context.Init(
        [&](vk::Instance instance) -> vk::SurfaceKHR {
            VkSurfaceKHR surface;
            if (!SDL_Vulkan_CreateSurface(mWindow, static_cast<VkInstance>(instance), nullptr, &surface))
            {
                LogE("Failed to create Vulkan surface: {}", SDL_GetError());
                throw std::runtime_error("Failed to create Vulkan surface");
            }
            return vk::SurfaceKHR(surface);
        },
        requiredExtensions);

    auto mIsRunning = true;
    while (mIsRunning)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                mIsRunning = false;
            }
        }
    }
    return 0;
}