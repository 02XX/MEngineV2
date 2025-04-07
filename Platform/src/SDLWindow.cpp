#include "SDLWindow.hpp"

namespace MEngine
{

SDLWindow::SDLWindow(std::shared_ptr<ILogger> logger, WindowConfig config) : mLogger(logger), mConfig(config)
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        mLogger->Error("Failed to initialize SDL: {}", SDL_GetError());
        throw std::runtime_error("Failed to initialize SDL");
    }
    mWindow =
        SDL_CreateWindow(mConfig.title.c_str(), config.width, config.height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
    if (!mWindow)
    {
        mLogger->Error("Failed to create window: {}", SDL_GetError());
        throw std::runtime_error("Failed to create window");
    }
    mLogger->Trace("SDL Window created successfully");
}
SDLWindow::~SDLWindow()
{
    if (mWindow)
    {
        SDL_DestroyWindow(mWindow);
    }
    SDL_Quit();
    mLogger->Trace("SDL Window destroyed");
}

vk::SurfaceKHR SDLWindow::GetSurface(vk::Instance instance) const
{
    VkSurfaceKHR surface;
    if (!SDL_Vulkan_CreateSurface(mWindow, static_cast<VkInstance>(instance), nullptr, &surface))
    {
        mLogger->Error("Failed to create Vulkan surface: {}", SDL_GetError());
        throw std::runtime_error("Failed to create Vulkan surface");
    }
    return vk::SurfaceKHR(surface);
}
std::vector<const char *> SDLWindow::GetInstanceRequiredExtensions() const
{
    Uint32 extensionCount = 0;
    auto extensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);
    std::vector<const char *> instanceRequiredExtensions(extensions, extensions + extensionCount);
    return instanceRequiredExtensions;
}
void SDLWindow::PollEvents()
{
    SDL_Event event;
    // 循环处理所有事件
    while (SDL_PollEvent(&event))
    {
        mEvent = event;
        // 触发所有回调
        for (auto &callback : mEventCallbacks)
        {
            callback(&event);
        }
        // 处理窗口关闭事件
        if (event.type == SDL_EVENT_QUIT)
        {
            mShouldClose = true;
        }
    }
}
void SDLWindow::SetEventCallback(EventCallback callback)
{
    mEventCallbacks.push_back(callback);
}
bool SDLWindow::ShouldClose() const
{
    return mShouldClose;
}

int SDLWindow::GetWidth() const
{
    return mWidth;
}
int SDLWindow::GetHeight() const
{
    return mHeight;
}
void *SDLWindow::GetNativeHandle() const
{
    return static_cast<void *>(mWindow);
}
} // namespace MEngine