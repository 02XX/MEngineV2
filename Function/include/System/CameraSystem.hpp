#pragma once
#include "Component/CameraComponent.hpp"
#include "Component/TransformComponent.hpp"
#include "Interface/ILogger.hpp"
#include "RenderPassManager.hpp"
#include "System.hpp"
#include "System/ISystem.hpp"
#include "entt/entt.hpp"
#include <memory>
namespace MEngine
{
class CameraSystem final : public System
{
  private:
    std::shared_ptr<RenderPassManager> mRenderPassManager;

  public:
    CameraSystem(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                 std::shared_ptr<IConfigure> configure, std::shared_ptr<entt::registry> registry,
                 std::shared_ptr<RenderPassManager> renderPassManager);
    void Init() override;
    void Tick(float deltaTime) override;
    void Shutdown() override;
};
} // namespace MEngine