#pragma once
#include "Componet/CameraComponent.hpp"
#include "Interface/ILogger.hpp"
#include "System/ISystem.hpp"
#include "entt/entt.hpp"
namespace MEngine
{
class CameraSystem final : public ISystem
{
  private:
    std::shared_ptr<ILogger> mLogger;
    std::shared_ptr<entt::registry> mRegistry;

  public:
    CameraSystem(std::shared_ptr<ILogger> logger, std::shared_ptr<entt::registry> registry);
    virtual void Init() override;
    virtual void Tick(float deltaTime) override;
    virtual void Shutdown() override;
};
} // namespace MEngine