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
#include "Entity/Texture2D.hpp"
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
#include "Repository/Texture2DRepository.hpp"
#include "SyncPrimitiveManager.hpp"
#include "System/CameraSystem.hpp"
#include "System/EditorRenderSystem.hpp"
#include "System/ISystem.hpp"
#include "System/InputSystem.hpp"
#include "System/RenderSystem.hpp"
#include "System/TransformSystem.hpp"


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
    std::shared_ptr<ILogger> mLogger;
    // std::shared_ptr<IConfigure> mConfigure;
    std::shared_ptr<IWindow> mWindow;
    std::shared_ptr<Context> mContext;
    std::shared_ptr<entt::registry> mRegistry;
    std::shared_ptr<BasicGeometryEntityManager> mBasicGeometryEntityManager;
    std::shared_ptr<ISystem> mRenderSystem;
    std::shared_ptr<ISystem> mCameraSystem;
    std::shared_ptr<ISystem> mTransformSystem;
    std::shared_ptr<ISystem> mInputSystem;

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
