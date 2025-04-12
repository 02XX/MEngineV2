#pragma once
#include "Componet/CameraComponent.hpp"
#include "Componet/MaterialComponent.hpp"
#include "Componet/MeshComponent.hpp"
#include "Componet/TransformComponent.hpp"
#include "System/System.hpp"
#include "entt/entt.hpp"
namespace MEngine
{
class TransformSystem final : public System
{
  private:
    // DI
    std::shared_ptr<ILogger> mLogger;
    std::shared_ptr<Context> mContext;
    std::shared_ptr<entt::registry> mRegistry;

  private:
    glm::mat4x4 mRotationMatrix = glm::mat4(1.0f);

  public:
    TransformSystem(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                    std::shared_ptr<entt::registry> registry);

    void Init() override;
    void Tick(float deltaTime) override;
    void Shutdown() override;
};
} // namespace MEngine