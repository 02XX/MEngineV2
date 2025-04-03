#include "Application.hpp"
#include "Context.hpp"
#include "ShaderManager.hpp"
#include <vector>

namespace MEngine
{
Application::Application()
{
    std::cout << "Application Started" << std::endl;
    // log
    Logger::Instance().Init();
    Logger::Instance().SetLevel(LogLevel::LOG_TRACE);
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
    std::vector<const char *> instanceRequiredExtensions(extensions, extensions + extensionCount);
    std::vector<const char *> deviceRequiredExtension;
    // instanceRequiredExtensions.push_back("VK_EXT_debug_utils");
    std::vector<const char *> instanceRequiredLayers{"VK_LAYER_KHRONOS_validation",
                                                     "VK_LAYER_KHRONOS_synchronization2"};
    deviceRequiredExtension.push_back("VK_KHR_swapchain");
#ifdef PLATFORM_MACOS
    instanceRequiredExtensions.push_back("VK_KHR_portability_enumeration");
    deviceRequiredExtension.push_back("VK_KHR_portability_subset");
#endif
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
        instanceRequiredExtensions, instanceRequiredLayers, deviceRequiredExtension, {""});

    // manager
    mRegistry = std::make_shared<entt::registry>();
    mShaderManager = std::make_shared<ShaderManager>();
    mPipelineLayoutManager = std::make_shared<PipelineLayoutManager>();
    mImageManager = std::make_shared<ImageManager>();
    mRenderPassManager = std::make_shared<RenderPassManager>(mImageManager);
    mPipelineManager = std::make_shared<PipelineManager>(mShaderManager, mPipelineLayoutManager, mRenderPassManager);
    mCommandBufferManager =
        std::make_shared<CommandBufferManager>(context.GetQueueFamilyIndicates().graphicsFamily.value());
    mSyncPrimitiveManager = std::make_shared<SyncPrimitiveManager>();
    mDescriptorManager = std::make_shared<DescriptorManager>();
    mSamplerManager = std::make_shared<SamplerManager>();
    mBufferManager = std::make_shared<BufferManager>();
    InitSystem();
    mBasicGeometryEntityManager = std::make_shared<BasicGeometryEntityManager>(mRegistry);
    mBasicGeometryEntityManager->CreateCube();
}
Application::~Application()
{
    // mCurrScene->OnDestroy();
    ShutdownSystem();
    SDL_DestroyWindow(mWindow);
    SDL_Quit();
    std::cout << "Application Closed" << std::endl;
}
void Application::InitSystem()
{
    mRenderSystem = std::make_shared<RenderSystem>(mWindow, mRegistry, mCommandBufferManager, mSyncPrimitiveManager,
                                                   mRenderPassManager, mPipelineLayoutManager, mPipelineManager);
    mRenderSystem->Init();
}
void Application::ShutdownSystem()
{
    mRenderSystem->Shutdown();
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
            mRenderSystem->Tick(1.0f);
        }
    }
}
} // namespace MEngine
