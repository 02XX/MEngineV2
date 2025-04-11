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
#include "TextureManager.hpp"
#include "boost/di.hpp"
#include "entt/entt.hpp"
#include <cstdint>
#include <memory>

namespace di = boost::di;
namespace MEngine
{
class Application final : public NoCopyable
{
  private:
    bool mIsRunning;
    // DI
    decltype(di::make_injector(
        di::bind<IConfigure>().to<Configure>().in(di::singleton), di::bind<ILogger>().to<SpdLogger>().in(di::singleton),
        di::bind<IWindow>().to<SDLWindow>().in(di::singleton), di::bind<Context>().to<Context>().in(di::singleton),
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
        di::bind<RenderSystem>().to<RenderSystem>().in(di::singleton))) mInjector;

    // DI
    std::shared_ptr<ILogger> mLogger;
    std::shared_ptr<IConfigure> mConfigure;
    std::shared_ptr<IWindow> mWindow;
    std::shared_ptr<Context> mContext;
    std::shared_ptr<entt::registry> mRegistry;
    std::shared_ptr<PipelineManager> mPipelineManager;
    std::shared_ptr<PipelineLayoutManager> mPipelineLayoutManager;
    std::shared_ptr<RenderPassManager> mRenderPassManager;
    std::shared_ptr<ShaderManager> mShaderManager;
    std::shared_ptr<CommandBufferManager> mCommandBufferManager;
    std::shared_ptr<SyncPrimitiveManager> mSyncPrimitiveManager;
    std::shared_ptr<ImageFactory> mImageFactory;
    std::shared_ptr<DescriptorManager> mDescriptorManager;
    std::shared_ptr<SamplerManager> mSamplerManager;
    std::shared_ptr<BufferFactory> mBufferFactory;
    std::shared_ptr<TextureManager> mTextureManager;
    std::shared_ptr<MaterialManager> mMaterialManager;
    std::shared_ptr<BasicGeometryFactory> mBasicGeometryFactory;
    std::shared_ptr<BasicGeometryEntityManager> mBasicGeometryEntityManager;
    std::shared_ptr<ISystem> mRenderSystem;
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
