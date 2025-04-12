#pragma once

#include "Application.hpp"
#include "BasicGeometry/BasicGeometryEntityManager.hpp"
#include "BufferFactory.hpp"
#include "Context.hpp"
#include "DescriptorManager.hpp"
#include "ImageFactory.hpp"
#include "Interface/ILogger.hpp"
#include "Interface/IWindow.hpp"
#include "NoCopyable.hpp"
#include "PipelineLayoutManager.hpp"
#include "PipelineManager.hpp"
#include "RenderPassManager.hpp"
#include "SDL3/SDL.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_vulkan.h"
#include "SDLWindow.hpp"
#include "SamplerManager.hpp"
#include "ShaderManager.hpp"
#include "SpdLogger.hpp"

#include "Configure.hpp"
#include "SyncPrimitiveManager.hpp"
#include "System/CameraSystem.hpp"
#include "System/ISystem.hpp"
#include "System/RenderSystem.hpp"
#include "System/UI.hpp"
#include "TextureManager.hpp"

#define BOOST_DI_CFG_CTOR_LIMIT_SIZE 30 // 定义构造函数参数的最大数量
#include "boost/di.hpp"
#include "entt/entt.hpp"
#include <cstdint>
#include <memory>

using namespace boost::di;
namespace MEngine
{
class Application final : public NoCopyable
{
  private:
    bool mIsRunning;
    // DI
    decltype(make_injector(
        bind<IConfigure>().to<Configure>().in(singleton), bind<ILogger>().to<SpdLogger>().in(singleton),
        bind<IWindow>().to<SDLWindow>().in(singleton), bind<Context>().to<Context>().in(singleton),
        bind<entt::registry>().to<entt::registry>().in(singleton),
        bind<CommandBufferManager>().to<CommandBufferManager>().in(singleton),
        bind<SyncPrimitiveManager>().to<SyncPrimitiveManager>().in(singleton),
        bind<PipelineLayoutManager>().to<PipelineLayoutManager>().in(singleton),
        bind<ShaderManager>().to<ShaderManager>().in(singleton),
        bind<DescriptorManager>().to<DescriptorManager>().in(singleton),
        bind<SamplerManager>().to<SamplerManager>().in(singleton),
        bind<BufferFactory>().to<BufferFactory>().in(singleton), bind<ImageFactory>().to<ImageFactory>().in(singleton),
        bind<RenderPassManager>().to<RenderPassManager>().in(singleton),
        bind<PipelineManager>().to<PipelineManager>().in(singleton),
        bind<TextureManager>().to<TextureManager>().in(singleton),
        bind<MaterialManager>().to<MaterialManager>().in(singleton),
        bind<BasicGeometryFactory>().to<BasicGeometryFactory>().in(singleton),
        bind<BasicGeometryEntityManager>().to<BasicGeometryEntityManager>().in(singleton),
        bind<CameraSystem>().to<CameraSystem>().in(singleton), bind<UI>().to<UI>().in(singleton),
        bind<RenderSystem>().to<RenderSystem>().in(singleton))) mInjector;

    // DI
    std::shared_ptr<ILogger> mLogger;
    // std::shared_ptr<IConfigure> mConfigure;
    std::shared_ptr<IWindow> mWindow;
    std::shared_ptr<Context> mContext;
    std::shared_ptr<entt::registry> mRegistry;
    // std::shared_ptr<PipelineManager> mPipelineManager;
    // std::shared_ptr<PipelineLayoutManager> mPipelineLayoutManager;
    // std::shared_ptr<RenderPassManager> mRenderPassManager;
    // std::shared_ptr<ShaderManager> mShaderManager;
    // std::shared_ptr<CommandBufferManager> mCommandBufferManager;
    // std::shared_ptr<SyncPrimitiveManager> mSyncPrimitiveManager;
    // std::shared_ptr<ImageFactory> mImageFactory;
    // std::shared_ptr<DescriptorManager> mDescriptorManager;
    // std::shared_ptr<SamplerManager> mSamplerManager;
    // std::shared_ptr<BufferFactory> mBufferFactory;
    // std::shared_ptr<TextureManager> mTextureManager;
    // std::shared_ptr<MaterialManager> mMaterialManager;
    // std::shared_ptr<BasicGeometryFactory> mBasicGeometryFactory;
    std::shared_ptr<BasicGeometryEntityManager> mBasicGeometryEntityManager;
    std::shared_ptr<UI> mUI;
    std::shared_ptr<RenderSystem> mRenderSystem;
    std::shared_ptr<ISystem> mCameraSystem;

    // time
    uint32_t mTargetFPS = 120;
    std::chrono::high_resolution_clock::time_point mStartTime;
    std::chrono::high_resolution_clock::time_point mCurrentTime;
    std::chrono::high_resolution_clock::time_point mLastTime;
    float mDeltaTime;

  private:
    void InitSystem();
    void ShutdownSystem();

  public:
    Application();
    ~Application();
    void Run();
};
} // namespace MEngine
