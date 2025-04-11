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

#include "SyncPrimitiveManager.hpp"
#include "System/CameraSystem.hpp"
#include "System/ISystem.hpp"
#include "System/RenderSystem.hpp"
#include "TextureManager.hpp"
#include "entt/entt.hpp"
#include <cstdint>
#include <memory>
namespace MEngine
{
class Application final : public NoCopyable
{
  private:
    bool mIsRunning;
    // DI
    std::shared_ptr<ILogger> mLogger;
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
    // System
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
