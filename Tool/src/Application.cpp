#include "Application.hpp"

namespace MEngine
{
auto injector = make_injector(
    DI::bind<IConfigure>().to<Configure>().in(DI::singleton), DI::bind<ILogger>().to<SpdLogger>().in(DI::singleton),
    DI::bind<IWindow>().to<SDLWindow>().in(DI::singleton), DI::bind<Context>().to<Context>().in(DI::singleton),
    DI::bind<entt::registry>().to<entt::registry>().in(DI::singleton),
    DI::bind<CommandBufferManager>().to<CommandBufferManager>().in(DI::singleton),
    DI::bind<SyncPrimitiveManager>().to<SyncPrimitiveManager>().in(DI::singleton),
    DI::bind<PipelineManager>().to<PipelineManager>().in(DI::singleton),
    DI::bind<PipelineLayoutManager>().to<PipelineLayoutManager>().in(DI::singleton),
    DI::bind<ShaderManager>().to<ShaderManager>().in(DI::singleton),
    DI::bind<DescriptorManager>().to<DescriptorManager>().in(DI::singleton),
    DI::bind<SamplerManager>().to<SamplerManager>().in(DI::singleton),
    DI::bind<BufferFactory>().to<BufferFactory>().in(DI::singleton),
    DI::bind<ImageFactory>().to<ImageFactory>().in(DI::singleton),
    DI::bind<RenderPassManager>().to<RenderPassManager>().in(DI::singleton),
    DI::bind<IRepository<Texture2D>>().to<Texture2DRepository>().in(DI::singleton),
    DI::bind<IRepository<PBRMaterial>>().to<PBRMaterialRepository>().in(DI::singleton),
    DI::bind<BasicGeometryFactory>().to<BasicGeometryFactory>().in(DI::singleton),
    DI::bind<BasicGeometryEntityManager>().to<BasicGeometryEntityManager>().in(DI::singleton),
    DI::bind<CameraSystem>().to<CameraSystem>().in(DI::singleton),
    DI::bind<RenderSystem>().to<EditorRenderSystem>().in(DI::singleton),
    DI::bind<TransformSystem>().to<TransformSystem>().in(DI::singleton),
    DI::bind<InputSystem>().to<InputSystem>().in(DI::singleton));

Application::Application()
{
    // DI
    // mConfigure = injector.create<std::shared_ptr<IConfigure>>();
    mLogger = injector.create<std::shared_ptr<ILogger>>();
    mLogger->Info("Application Started");
    mWindow = injector.create<std::shared_ptr<IWindow>>();
    mContext = injector.create<std::shared_ptr<Context>>();
    mRegistry = injector.create<std::shared_ptr<entt::registry>>();
    mBasicGeometryEntityManager = injector.create<std::shared_ptr<BasicGeometryEntityManager>>();

    mBasicGeometryEntityManager->CreateCube(mRegistry);
    auto camera = mRegistry->create();
    mRegistry->emplace<TransformComponent>(
        camera, TransformComponent(glm::vec3(0.0f, 0.0f, 5.0f), glm::quat(), glm::vec3(1.0f, 1.0f, 1.0f)));
    mRegistry->emplace<CameraComponent>(camera).isMainCamera = false;
    mRegistry->emplace<InputComponent>(camera);
    auto light = mRegistry->create();
    mRegistry->emplace<TransformComponent>(light);

    mRegistry->emplace<LightComponent>(light, LightComponent{});

    InitSystem();
    mStartTime = std::chrono::high_resolution_clock::now();
    mLastTime = mStartTime;
}
Application::~Application()
{
    mContext->GetDevice().waitIdle();
    mLogger->Info("Application Closed");
}
void Application::InitSystem()
{
    mRenderSystem = injector.create<std::shared_ptr<RenderSystem>>();
    mCameraSystem = injector.create<std::shared_ptr<CameraSystem>>();
    mTransformSystem = injector.create<std::shared_ptr<TransformSystem>>();
    mInputSystem = injector.create<std::shared_ptr<InputSystem>>();
    mRenderSystem->Init();
    mCameraSystem->Init();
    mTransformSystem->Init();
    mInputSystem->Init();
}
void Application::ShutdownSystem()
{
    mInputSystem->Shutdown();
    mTransformSystem->Shutdown();
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
        mInputSystem->Tick(mDeltaTime);
        mCameraSystem->Tick(mDeltaTime);
        mTransformSystem->Tick(mDeltaTime);
        mRenderSystem->Tick(mDeltaTime);
        if (mDeltaTime < 1.0f / mTargetFPS)
        {
            auto sleepTime = (1.0f / mTargetFPS - mDeltaTime);
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(sleepTime * 1000)));
        }
    }
}
} // namespace MEngine
