#include "BasicGeometry/BasicGeometryEntityManager.hpp"
#include "PipelineLayoutManager.hpp"
#include "PipelineManager.hpp"

namespace MEngine
{
BasicGeometryEntityManager::BasicGeometryEntityManager(std::shared_ptr<entt::registry> registry,
                                                       std::shared_ptr<BufferManager> bufferManager)
    : mRegistry(registry), mBufferManager(bufferManager)
{
    mBasicGeometryManager = std::make_unique<BasicGeometryManager>(bufferManager);
    // mMaterialManager = std::make_unique<MaterialManager>();
}
entt::entity BasicGeometryEntityManager::CreateCube()
{
    auto mesh = mBasicGeometryManager->GetPrimitive(PrimitiveType::Cube);
    auto entity = mRegistry->create();
    mRegistry->emplace<TransformComponent>(entity, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                                           glm::vec3(1.0f, 1.0f, 1.0f));
    mRegistry->emplace<MeshComponent>(entity, mesh);
    mRegistry->emplace<MaterialComponent>(entity, PipelineType::Translucency, PipelineLayoutType::TranslucencyLayout);
    return entity;
}
entt::entity BasicGeometryEntityManager::CreateQuad()
{
    auto mesh = mBasicGeometryManager->GetPrimitive(PrimitiveType::Quad);
    auto entity = mRegistry->create();
    mRegistry->emplace<TransformComponent>(entity, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                                           glm::vec3(1.0f, 1.0f, 1.0f));
    // mRegistry->emplace<MeshComponent>(entity, mesh);
    // auto defaultMaterial = mMaterialManager->GetMaterial(MaterialType::Default);
    // mRegistry->emplace<MaterialComponent>(entity, defaultMaterial);

    return entity;
}
entt::entity BasicGeometryEntityManager::CreateCylinder()
{
    auto mesh = mBasicGeometryManager->GetPrimitive(PrimitiveType::Cylinder);
    auto entity = mRegistry->create();
    mRegistry->emplace<TransformComponent>(entity, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                                           glm::vec3(1.0f, 1.0f, 1.0f));
    // mRegistry->emplace<MeshComponent>(entity, mesh);
    // auto defaultMaterial = mMaterialManager->GetMaterial(MaterialType::Default);
    // mRegistry->emplace<MaterialComponent>(entity, defaultMaterial);
    return entity;
}
entt::entity BasicGeometryEntityManager::CreateSphere()
{
    auto mesh = mBasicGeometryManager->GetPrimitive(PrimitiveType::Sphere);
    auto entity = mRegistry->create();
    mRegistry->emplace<TransformComponent>(entity, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                                           glm::vec3(1.0f, 1.0f, 1.0f));
    // mRegistry->emplace<MeshComponent>(entity, mesh);
    // auto defaultMaterial = mMaterialManager->GetMaterial(MaterialType::Default);
    // mRegistry->emplace<MaterialComponent>(entity, defaultMaterial);
    return entity;
}

} // namespace MEngine