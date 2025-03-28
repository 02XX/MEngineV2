#include "Application.hpp"

namespace MEngine
{
Application::Application()
{
    std::cout << "Application Started" << std::endl;
    // log
    Logger::Instance().SetLevel(LogLevel::TRACE);
    // sdl
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        LogE("Failed to initialize SDL: {}", SDL_GetError());
    }
    mWindow = SDL_CreateWindow("MEngine", 800, 600, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
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

    // mCurrScene = std::make_shared<DefaultScene>();
    // mCurrScene->OnCreate();
}
Application::~Application()
{
    // mCurrScene->OnDestroy();
    SDL_DestroyWindow(mWindow);
    SDL_Quit();
    std::cout << "Application Closed" << std::endl;
}
void Application::Run()
{
    mIsRunning = true;
    while (mIsRunning)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                mIsRunning = false;
            }
            // mCurrScene->HandleEvent(event);
            // mCurrScene->Update();
            // mCurrScene->LateUpdate();
            // mCurrScene->Draw();
        }
    }
}
} // namespace MEngine
