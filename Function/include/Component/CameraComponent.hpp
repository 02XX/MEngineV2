#pragma once
#include "Component/Interface/IComponent.hpp"
#include "MEngine.hpp"
#include "Math.hpp"
#include "entt/entity/storage.hpp"
namespace MEngine
{
struct CameraComponent : public IComponent<>
{
    bool isMainCamera = false;
    float aspectRatio = 1;
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 10.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
    float yaw = 0;
    float pitch = 0;
    float roll = 0;
    float zoom = 1;
    float fovY = 60;
    float nearPlane = 0.1;
    float farPlane = 100;

    // View Matrix
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    // Projection Matrix
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    CameraComponent()
    {
    }
    CameraComponent(bool isMainCamera, float aspectRatio, glm::vec3 position, glm::vec3 up, glm::vec3 front, float yaw,
                    float pitch, float roll, float zoom, float fovY, float nearPlane, float farPlane)
        : isMainCamera(isMainCamera), aspectRatio(aspectRatio), position(position), up(up), front(front), yaw(yaw),
          pitch(pitch), roll(roll), zoom(zoom), fovY(fovY), nearPlane(nearPlane), farPlane(farPlane)
    {
    }
};
} // namespace MEngine
// template <> struct entt::storage_type<MEngine::CameraComponent>
// {
//     using type = entt::storage<MEngine::CameraComponent>;
// };