#include "System/CameraSystem.hpp"
#include "Component/InputComponent.hpp"
#include "SDL3/SDL_oldnames.h"

namespace MEngine
{
CameraSystem::CameraSystem(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                           std::shared_ptr<IConfigure> configure, std::shared_ptr<entt::registry> registry,
                           std::shared_ptr<RenderPassManager> renderPassManager)
    : System(logger, context, configure, registry), mRenderPassManager(renderPassManager)
{
}
void CameraSystem::Init()
{
    auto view = mRegistry->view<CameraComponent, InputComponent>();
    for (auto entity : view)
    {
        auto &inputComponent = view.get<InputComponent>(entity);
        auto &camera = view.get<CameraComponent>(entity);

        inputComponent.eventCallback = [&](const SDL_Event *event, float deltaTime) {
            bool rightMouseDown = false;
            if (event->type == SDL_EVENT_KEY_DOWN)
            {
                if (event->key.repeat)
                {
                    switch (event->key.key)
                    {
                    case SDLK_W:
                        camera.position.y += 0.1f;
                        break;
                    case SDLK_S:
                        camera.position.y -= 0.1f;
                        break;
                    case SDLK_A:
                        camera.position.x -= 0.1f;
                        break;
                    case SDLK_D:
                        camera.position.x += 0.1f;
                        break;
                    }
                }
            }
            if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN)
            {
                if (event->button.button == SDL_BUTTON_RIGHT)
                {
                    rightMouseDown = true;
                }
            }
            if (event->type == SDL_EVENT_MOUSE_BUTTON_UP)
            {
                if (event->button.button == SDL_BUTTON_RIGHT)
                {
                    rightMouseDown = false;
                }
            }
            if (event->type == SDL_EVENT_MOUSE_MOTION && rightMouseDown)
            {
                camera.yaw += event->motion.xrel * camera.moveSpeed * deltaTime;
                camera.pitch -= event->motion.yrel * camera.moveSpeed * deltaTime;
                camera.pitch = std::clamp(camera.pitch, -89.0f, 89.0f);
            }
            if (event->type == SDL_EVENT_MOUSE_WHEEL)
            {
                camera.zoom += event->wheel.y * camera.zoomSpeed * deltaTime;
                camera.zoom = std::clamp(camera.zoom, 0.1f, 10.0f); // 限制缩放范围
            }
        };
    }
    mLogger->Info("Camera System Initialized");
}
void CameraSystem::Tick(float deltaTime)
{
    auto entities = mRegistry->view<CameraComponent>();
    for (auto entity : entities)
    {
        auto &camera = entities.get<CameraComponent>(entity);
        auto extent = mRenderPassManager->GetRenderTargetExtent();

        camera.aspectRatio = static_cast<float>(extent.width * 1.0f) / extent.height;
        glm::mat4 viewMatrix = glm::lookAtRH(camera.position, camera.position + camera.front, camera.up);
        glm::mat4 pitchMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(camera.pitch), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 yawMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(camera.yaw), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 rollMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(camera.roll), glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 rotationMatrix = yawMatrix * pitchMatrix * rollMatrix;
        camera.viewMatrix = rotationMatrix * viewMatrix;
        camera.projectionMatrix = glm::perspective(glm::radians(camera.fovY / camera.zoom), camera.aspectRatio,
                                                   camera.nearPlane, camera.farPlane);
    }
}
void CameraSystem::Shutdown()
{
    mLogger->Info("Camera System Shutdown");
}
} // namespace MEngine