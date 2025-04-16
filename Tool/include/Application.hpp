#pragma once

#include "Application.hpp"
#include "BasicGeometry/BasicGeometryEntityManager.hpp"
#include "BufferFactory.hpp"
#include "Component/CameraComponent.hpp"
#include "Component/MaterialComponent.hpp"
#include "Component/TransformComponent.hpp"
#include "Context.hpp"
#include "DescriptorManager.hpp"
#include "Entity/Interface/IMaterial.hpp"
#include "Entity/Interface/ITexture.hpp"
#include "Entity/PBRMaterial.hpp"
#include "Entity/Texture.hpp"
#include "ImageFactory.hpp"
#include "Interface/ILogger.hpp"
#include "Interface/IWindow.hpp"
#include "NoCopyable.hpp"
#include "PipelineLayoutManager.hpp"
#include "PipelineManager.hpp"
#include "RenderPassManager.hpp"
#include "Repository/Interface/IRepository.hpp"
#include "Repository/PBRMaterialRepository.hpp"
#include "SDL3/SDL.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_vulkan.h"
#include "SDLWindow.hpp"
#include "SamplerManager.hpp"
#include "ShaderManager.hpp"
#include "SpdLogger.hpp"

#include "Configure.hpp"
#include "Repository/TextureRepository.hpp"
#include "SyncPrimitiveManager.hpp"
#include "System/CameraSystem.hpp"
#include "System/ISystem.hpp"
#include "System/RenderSystem.hpp"
#include "System/TransformSystem.hpp"
#include "System/UI.hpp"

#define BOOST_DI_CFG_CTOR_LIMIT_SIZE 50 // 定义构造函数参数的最大数量
#include "boost/di.hpp"
#include "entt/entt.hpp"
#include <cstdint>
#include <memory>

namespace DI = boost::di;
namespace MEngine
{
class Application final : public NoCopyable
{
  private:
    bool mIsRunning;
    // DI
    decltype(make_injector(
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
        DI::bind<IRepository<Texture>>().to<TextureRepository>().in(DI::singleton),
        DI::bind<IRepository<PBRMaterial>>().to<PBRMaterialRepository>().in(DI::singleton),
        DI::bind<BasicGeometryFactory>().to<BasicGeometryFactory>().in(DI::singleton),
        DI::bind<BasicGeometryEntityManager>().to<BasicGeometryEntityManager>().in(DI::singleton),
        DI::bind<CameraSystem>().to<CameraSystem>().in(DI::singleton), DI::bind<UI>().to<UI>().in(DI::singleton),
        DI::bind<RenderSystem>().to<RenderSystem>().in(DI::singleton),
        DI::bind<TransformSystem>().to<TransformSystem>().in(DI::singleton))) mInjector;

    // DI
    std::shared_ptr<ILogger> mLogger;
    // std::shared_ptr<IConfigure> mConfigure;
    std::shared_ptr<IWindow> mWindow;
    std::shared_ptr<Context> mContext;
    std::shared_ptr<entt::registry> mRegistry;
    std::shared_ptr<BasicGeometryEntityManager> mBasicGeometryEntityManager;
    std::shared_ptr<UI> mUI;
    std::shared_ptr<ISystem<MaterialComponent, MeshComponent>> mRenderSystem;
    std::shared_ptr<ISystem<CameraComponent>> mCameraSystem;
    std::shared_ptr<ISystem<TransformComponent>> mTransformSystem;

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
