#pragma once

#include "Context.hpp"
#include "IScene.hpp"
#include "Logger.hpp"
#include "MEngine.hpp"
#include "PipelineLayoutManager.hpp"
#include "PipelineManager.hpp"
#include "RenderPassManager.hpp"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_video.h"
#include "SDL3/SDL_vulkan.h"
#include "SamplerManager.hpp"
#include "Scene/DefaultScene.hpp"
#include "ShaderManager.hpp"
#include <memory>

namespace MEngine
{
class Application final
{
  private:
    bool mIsRunning;
    SDL_Window *mWindow;
    std::shared_ptr<IScene> mCurrScene;
    // manager
    std::shared_ptr<entt::registry> mRegistry;
    std::shared_ptr<RenderSystem> mRenderSystem;
    std::shared_ptr<PipelineManager> mPipelineManager;
    std::shared_ptr<PipelineLayoutManager> mPipelineLayoutManager;
    std::shared_ptr<RenderPassManager> mRenderPassManager;
    std::shared_ptr<ShaderManager> mShaderManager;
    std::shared_ptr<CommandBufferManager> mCommandBufferManager;
    std::shared_ptr<SyncPrimitiveManager> mSyncPrimitiveManager;
    std::shared_ptr<ImageManager> mImageManager;
    std::shared_ptr<DescriptorManager> mDescriptorManager;
    std::shared_ptr<SamplerManager> mSamplerManager;
    std::shared_ptr<BufferManager> mBufferManager;

  private:
    void InitSystem();
    void ShutdownSystem();

  public:
    Application();
    ~Application();
    void Run();
};
} // namespace MEngine
