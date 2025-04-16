#pragma once
#include "Component/CameraComponent.hpp"
#include "Component/TransformComponent.hpp"
#include "Interface/ILogger.hpp"
#include "System.hpp"
#include "System/ISystem.hpp"
#include "entt/entt.hpp"
namespace MEngine
{
class CameraSystem final : public System<CameraComponent>
{
  public:
    CameraSystem(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                 std::shared_ptr<IConfigure> configure, std::shared_ptr<entt::registry> registry);
    void Init() override;
    void Tick(float deltaTime) override;
    void Shutdown() override;
};
} // namespace MEngine