#include "System/CameraSystem.hpp"

namespace MEngine
{
CameraSystem::CameraSystem(std::shared_ptr<ILogger> logger, std::shared_ptr<entt::registry> registry)
    : mLogger(logger), mRegistry(registry)
{
}
void CameraSystem::Init()
{
    mLogger->Info("Camera System Initialized");
}
void CameraSystem::Tick(float deltaTime)
{
    auto entities = mRegistry->view<CameraComponent>();
    for (auto entity : entities)
    {
        auto &camera = entities.get<CameraComponent>(entity);
        glm::mat4 viewMatrix = glm::lookAtRH(camera.position, camera.position + camera.front, camera.up);
        glm::mat4 pitchMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(camera.pitch), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 yawMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(camera.yaw), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 rollMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(camera.roll), glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 rotationMatrix = yawMatrix * pitchMatrix * rollMatrix;
        camera.viewMatrix = rotationMatrix * viewMatrix;
        camera.projectionMatrix =
            glm::perspective(glm::radians(camera.fovY), camera.aspectRatio, camera.nearPlane, camera.farPlane);
    }
}
void CameraSystem::Shutdown()
{
    mLogger->Info("Camera System Shutdown");
}
} // namespace MEngine