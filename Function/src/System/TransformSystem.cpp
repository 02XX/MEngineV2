#include "System/TransformSystem.hpp"

namespace MEngine
{
TransformSystem::TransformSystem(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                                 std::shared_ptr<IConfigure> configure, std::shared_ptr<entt::registry> registry)
    : System(logger, context, configure, registry)
{
}
void TransformSystem::Init()
{
    mLogger->Info("Transform System Init");
}
void TransformSystem::Shutdown()
{
    mLogger->Info("Transform System Shutdown");
}
void TransformSystem::Tick(float deltaTime)
{
    // 1. 更新旋转矩阵
    mRotationMatrix = glm::rotate(mRotationMatrix, glm::radians(60.f) * deltaTime, glm::vec3(0.0f, 1.0f, 0.0f));
    auto view = mRegistry->view<TransformComponent>();
    for (auto entity : view)
    {
        auto &transform = view.get<TransformComponent>(entity);
        glm::mat4x4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, transform.position);
        modelMatrix = modelMatrix * glm::mat4_cast(transform.rotation);
        modelMatrix = glm::scale(modelMatrix, transform.scale);

        transform.modelMatrix = modelMatrix;
    }
}

} // namespace MEngine