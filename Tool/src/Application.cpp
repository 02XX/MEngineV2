#include "Application.hpp"

namespace MEngine
{
Application::Application()
{
    // DI
    mLogger = std::make_shared<SpdLogger>();
    mLogger->Info("Application Started");

    mWindow = std::make_shared<SDLWindow>(mLogger, WindowConfig{1400, 800, "MEngine"});
    std::vector<const char *> instanceRequiredExtensions = mWindow->GetInstanceRequiredExtensions();
    std::vector<const char *> instanceRequiredLayers{"VK_LAYER_KHRONOS_validation",
                                                     "VK_LAYER_KHRONOS_synchronization2"};
    std::vector<const char *> deviceRequiredExtension;
    std::vector<const char *> deviceRequiredLayers;
    deviceRequiredExtension.push_back("VK_KHR_swapchain");
#ifdef PLATFORM_MACOS
    instanceRequiredExtensions.push_back("VK_KHR_portability_enumeration");
    deviceRequiredExtension.push_back("VK_KHR_portability_subset");
#endif
    mContext = std::make_shared<Context>(mLogger, mWindow,
                                         ContextConfig{instanceRequiredExtensions, instanceRequiredLayers,
                                                       deviceRequiredExtension, deviceRequiredLayers});
    mRegistry = std::make_shared<entt::registry>();
    mCommandBufferManager = std::make_shared<CommandBufferManager>(mLogger, mContext);
    mSyncPrimitiveManager = std::make_shared<SyncPrimitiveManager>(mLogger, mContext);
    mPipelineLayoutManager = std::make_shared<PipelineLayoutManager>(mLogger, mContext);
    mShaderManager = std::make_shared<ShaderManager>(mLogger, mContext);
    mDescriptorManager = std::make_shared<DescriptorManager>(mLogger, mContext);
    mSamplerManager = std::make_shared<SamplerManager>(mLogger, mContext);
    mBufferManager = std::make_shared<BufferManager>(mLogger, mContext, mCommandBufferManager, mSyncPrimitiveManager);
    mDescriptorManager = std::make_shared<DescriptorManager>(mLogger, mContext);
    mImageManager =
        std::make_shared<ImageManager>(mLogger, mContext, mCommandBufferManager, mSyncPrimitiveManager, mBufferManager);

    mRenderPassManager = std::make_shared<RenderPassManager>(mLogger, mContext, mImageManager);
    mPipelineManager = std::make_shared<PipelineManager>(mLogger, mContext, mShaderManager, mPipelineLayoutManager,
                                                         mRenderPassManager);

    mBasicGeometryEntityManager = std::make_shared<BasicGeometryEntityManager>(
        mContext, mLogger, mBufferManager, mPipelineManager, mPipelineLayoutManager, mDescriptorManager);
    mBasicGeometryEntityManager->CreateCube(mRegistry);
    // Camera
    auto camera = mRegistry->create();
    mRegistry->emplace<CameraComponent>(camera).isMainCamera = true;

    InitSystem();

    mStartTime = std::chrono::high_resolution_clock::now();
    mLastTime = mStartTime;
}
Application::~Application()
{
    mLogger->Info("Application Closed");
}
void Application::InitSystem()
{
    mRenderSystem = std::make_shared<RenderSystem>(mLogger, mContext, mWindow, mRegistry, mCommandBufferManager,
                                                   mSyncPrimitiveManager, mRenderPassManager, mPipelineLayoutManager,
                                                   mPipelineManager, mDescriptorManager);
    mCameraSystem = std::make_shared<CameraSystem>(mLogger, mRegistry);
    mRenderSystem->Init();
    mCameraSystem->Init();
}
void Application::ShutdownSystem()
{
    mCameraSystem->Shutdown();
    mRenderSystem->Shutdown();
}
void Application::Run()
{
    mIsRunning = true;
    while (mIsRunning)
    {
        if (mWindow->ShouldClose())
        {
            mIsRunning = false;
            break;
        }
        mLastTime = std::chrono::high_resolution_clock::now();
        mDeltaTime = std::chrono::duration<float>(mLastTime - mStartTime).count();
        mWindow->PollEvents();
        mCameraSystem->Tick(mDeltaTime);
        mRenderSystem->Tick(mDeltaTime);
    }
}
} // namespace MEngine
