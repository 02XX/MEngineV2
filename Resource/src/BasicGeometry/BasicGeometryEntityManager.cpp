#include "BasicGeometry/BasicGeometryEntityManager.hpp"
namespace MEngine
{
BasicGeometryEntityManager::BasicGeometryEntityManager(std::shared_ptr<Context> context,
                                                       std::shared_ptr<ILogger> logger,
                                                       std::shared_ptr<BufferManager> bufferManager,
                                                       std::shared_ptr<PipelineManager> pipelineManager,
                                                       std::shared_ptr<PipelineLayoutManager> pipelineLayoutManager,
                                                       std::shared_ptr<DescriptorManager> descriptorManager)
    : mContext(context), mLogger(logger), mBufferManager(bufferManager), mPipelineManager(pipelineManager),
      mPipelineLayoutManager(pipelineLayoutManager), mDescriptorManager(descriptorManager)
{
    mBasicGeometryManager = std::make_unique<BasicGeometryManager>(bufferManager);
}
entt::entity BasicGeometryEntityManager::CreateCube(std::shared_ptr<entt::registry> registry)
{
    auto mesh = mBasicGeometryManager->GetPrimitive(PrimitiveType::Cube);
    auto material = std::make_shared<Material>(mDescriptorManager, mPipelineManager, mPipelineLayoutManager,
                                               PipelineType::Translucency, PipelineLayoutType::TranslucencyLayout);
    auto entity = registry->create();
    registry->emplace<TransformComponent>(entity, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                                          glm::vec3(1.0f, 1.0f, 1.0f));
    registry->emplace<MeshComponent>(entity, mesh);
    registry->emplace<MaterialComponent>(entity, material);
    return entity;
}
entt::entity BasicGeometryEntityManager::CreateQuad(std::shared_ptr<entt::registry> registry)
{
    auto mesh = mBasicGeometryManager->GetPrimitive(PrimitiveType::Quad);
    auto entity = registry->create();
    registry->emplace<TransformComponent>(entity, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                                          glm::vec3(1.0f, 1.0f, 1.0f));
    // mRegistry->emplace<MeshComponent>(entity, mesh);
    // auto defaultMaterial = mMaterialManager->GetMaterial(MaterialType::Default);
    // mRegistry->emplace<MaterialComponent>(entity, defaultMaterial);

    return entity;
}
entt::entity BasicGeometryEntityManager::CreateCylinder(std::shared_ptr<entt::registry> registry)
{
    auto mesh = mBasicGeometryManager->GetPrimitive(PrimitiveType::Cylinder);
    auto entity = registry->create();
    registry->emplace<TransformComponent>(entity, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                                          glm::vec3(1.0f, 1.0f, 1.0f));
    // mRegistry->emplace<MeshComponent>(entity, mesh);
    // auto defaultMaterial = mMaterialManager->GetMaterial(MaterialType::Default);
    // mRegistry->emplace<MaterialComponent>(entity, defaultMaterial);
    return entity;
}
entt::entity BasicGeometryEntityManager::CreateSphere(std::shared_ptr<entt::registry> registry)
{
    auto mesh = mBasicGeometryManager->GetPrimitive(PrimitiveType::Sphere);
    auto entity = registry->create();
    registry->emplace<TransformComponent>(entity, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                                          glm::vec3(1.0f, 1.0f, 1.0f));
    // mRegistry->emplace<MeshComponent>(entity, mesh);
    // auto defaultMaterial = mMaterialManager->GetMaterial(MaterialType::Default);
    // mRegistry->emplace<MaterialComponent>(entity, defaultMaterial);
    return entity;
}

} // namespace MEngine