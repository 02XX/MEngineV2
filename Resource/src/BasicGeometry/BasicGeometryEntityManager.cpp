#include "BasicGeometry/BasicGeometryEntityManager.hpp"

namespace MEngine
{
entt::entity BasicGeometryEntityManager::CreateQuad()
{
    auto mesh = mBasicGeometryManager->GetPrimitive(PrimitiveType::Quad);
    auto entity = mRegistry->create();
    mRegistry->emplace<TransformComponent>(entity, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                                           glm::vec3(1.0f, 1.0f, 1.0f));
    mRegistry->emplace<MeshComponent>(entity, mesh);
    
    return entity;
}
} // namespace MEngine