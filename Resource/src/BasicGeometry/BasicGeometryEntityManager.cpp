#include "BasicGeometry/BasicGeometryEntityManager.hpp"

namespace MEngine
{
BasicGeometryEntityManager::BasicGeometryEntityManager(std::shared_ptr<entt::registry> registry) : mRegistry(registry)
{
    mBasicGeometryManager = std::make_unique<BasicGeometryManager>();
    mMaterialManager = std::make_unique<MaterialManager>();
}
entt::entity BasicGeometryEntityManager::CreateCube()
{
    auto mesh = mBasicGeometryManager->GetPrimitive(PrimitiveType::Cube);
    auto entity = mRegistry->create();
    mRegistry->emplace<TransformComponent>(entity, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                                           glm::vec3(1.0f, 1.0f, 1.0f));
    mRegistry->emplace<MeshComponent>(entity, mesh);
    auto pbrMaterial = mMaterialManager->GetMaterial(MaterialType::PBR);
    mRegistry->emplace<MaterialComponent>(entity, pbrMaterial);

    return entity;
}
entt::entity BasicGeometryEntityManager::CreateQuad()
{
    auto mesh = mBasicGeometryManager->GetPrimitive(PrimitiveType::Quad);
    auto entity = mRegistry->create();
    mRegistry->emplace<TransformComponent>(entity, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                                           glm::vec3(1.0f, 1.0f, 1.0f));
    mRegistry->emplace<MeshComponent>(entity, mesh);
    auto defaultMaterial = mMaterialManager->GetMaterial(MaterialType::Default);
    mRegistry->emplace<MaterialComponent>(entity, defaultMaterial);

    return entity;
}
entt::entity BasicGeometryEntityManager::CreateCylinder()
{
    auto mesh = mBasicGeometryManager->GetPrimitive(PrimitiveType::Cylinder);
    auto entity = mRegistry->create();
    mRegistry->emplace<TransformComponent>(entity, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                                           glm::vec3(1.0f, 1.0f, 1.0f));
    mRegistry->emplace<MeshComponent>(entity, mesh);
    auto defaultMaterial = mMaterialManager->GetMaterial(MaterialType::Default);
    mRegistry->emplace<MaterialComponent>(entity, defaultMaterial);
    return entity;
}
entt::entity BasicGeometryEntityManager::CreateSphere()
{
    auto mesh = mBasicGeometryManager->GetPrimitive(PrimitiveType::Sphere);
    auto entity = mRegistry->create();
    mRegistry->emplace<TransformComponent>(entity, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                                           glm::vec3(1.0f, 1.0f, 1.0f));
    mRegistry->emplace<MeshComponent>(entity, mesh);
    auto defaultMaterial = mMaterialManager->GetMaterial(MaterialType::Default);
    mRegistry->emplace<MaterialComponent>(entity, defaultMaterial);
    return entity;
}

} // namespace MEngine