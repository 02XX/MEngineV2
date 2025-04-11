#include "Application.hpp"

namespace MEngine
{
Application::Application()
    : mInjector(di::make_injector(
          di::bind<IConfigure>().to<Configure>().in(di::singleton),
          di::bind<ILogger>().to<SpdLogger>().in(di::singleton), di::bind<IWindow>().to<SDLWindow>().in(di::singleton),
          di::bind<Context>().to<Context>().in(di::singleton),
          di::bind<entt::registry>().to<entt::registry>().in(di::singleton),
          di::bind<CommandBufferManager>().to<CommandBufferManager>().in(di::singleton),
          di::bind<SyncPrimitiveManager>().to<SyncPrimitiveManager>().in(di::singleton),
          di::bind<PipelineLayoutManager>().to<PipelineLayoutManager>().in(di::singleton),
          di::bind<ShaderManager>().to<ShaderManager>().in(di::singleton),
          di::bind<DescriptorManager>().to<DescriptorManager>().in(di::singleton),
          di::bind<SamplerManager>().to<SamplerManager>().in(di::singleton),
          di::bind<BufferFactory>().to<BufferFactory>().in(di::singleton),
          di::bind<ImageFactory>().to<ImageFactory>().in(di::singleton),
          di::bind<RenderPassManager>().to<RenderPassManager>().in(di::singleton),
          di::bind<PipelineManager>().to<PipelineManager>().in(di::singleton),
          di::bind<TextureManager>().to<TextureManager>().in(di::singleton),
          di::bind<MaterialManager>().to<MaterialManager>().in(di::singleton),
          di::bind<BasicGeometryFactory>().to<BasicGeometryFactory>().in(di::singleton),
          di::bind<BasicGeometryEntityManager>().to<BasicGeometryEntityManager>().in(di::singleton),
          di::bind<CameraSystem>().to<CameraSystem>().in(di::singleton),
          di::bind<RenderSystem>().to<RenderSystem>().in(di::singleton)))
{
    // DI
    mConfigure = mInjector.create<std::shared_ptr<IConfigure>>();
    mLogger = mInjector.create<std::shared_ptr<ILogger>>();
    mLogger->Info("Application Started");
    mWindow = mInjector.create<std::shared_ptr<IWindow>>();
    mContext = mInjector.create<std::shared_ptr<Context>>();
    mRegistry = mInjector.create<std::shared_ptr<entt::registry>>();
    mCommandBufferManager = mInjector.create<std::shared_ptr<CommandBufferManager>>();
    mSyncPrimitiveManager = mInjector.create<std::shared_ptr<SyncPrimitiveManager>>();
    mPipelineLayoutManager = mInjector.create<std::shared_ptr<PipelineLayoutManager>>();
    mShaderManager = mInjector.create<std::shared_ptr<ShaderManager>>();
    mDescriptorManager = mInjector.create<std::shared_ptr<DescriptorManager>>();
    mSamplerManager = mInjector.create<std::shared_ptr<SamplerManager>>();
    mBufferFactory = mInjector.create<std::shared_ptr<BufferFactory>>();
    mDescriptorManager = mInjector.create<std::shared_ptr<DescriptorManager>>();
    mImageFactory = mInjector.create<std::shared_ptr<ImageFactory>>();
    mRenderPassManager = mInjector.create<std::shared_ptr<RenderPassManager>>();
    mPipelineManager = mInjector.create<std::shared_ptr<PipelineManager>>();
    mTextureManager = mInjector.create<std::shared_ptr<TextureManager>>();
    mMaterialManager = mInjector.create<std::shared_ptr<MaterialManager>>();
    mBasicGeometryFactory = mInjector.create<std::shared_ptr<BasicGeometryFactory>>();
    mBasicGeometryEntityManager = mInjector.create<std::shared_ptr<BasicGeometryEntityManager>>();

    mBasicGeometryEntityManager->CreateCube(mRegistry);
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
    mRenderSystem =
        std::make_shared<RenderSystem>(mLogger, mContext, mRegistry, mRenderPassManager, mPipelineLayoutManager,
                                       mPipelineManager, mCommandBufferManager, mSyncPrimitiveManager,
                                       mDescriptorManager, mSamplerManager, mBufferFactory, mImageFactory, mWindow);
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
        mCurrentTime = std::chrono::high_resolution_clock::now();
        auto elapsedTime = mCurrentTime - mLastTime;
        mDeltaTime = std::chrono::duration<float>(elapsedTime).count();
        mLastTime = mCurrentTime;

        mWindow->PollEvents();
        mCameraSystem->Tick(mDeltaTime);
        mRenderSystem->Tick(mDeltaTime);

        if (mDeltaTime < 1.0f / mTargetFPS)
        {
            auto sleepTime = (1.0f / mTargetFPS - mDeltaTime);
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(sleepTime * 1000)));
        }
    }
}
} // namespace MEngine
