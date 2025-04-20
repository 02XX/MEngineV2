#include "BasicGeometry/BasicGeometryEntityManager.hpp"

namespace MEngine
{
BasicGeometryEntityManager::BasicGeometryEntityManager(
    std::shared_ptr<ILogger> mLogger, std::shared_ptr<Context> context,
    std::shared_ptr<PipelineManager> pipelineManager, std::shared_ptr<PipelineLayoutManager> pipelineLayoutManager,
    std::shared_ptr<DescriptorManager> descriptorManager, std::shared_ptr<SamplerManager> samplerManager,
    std::shared_ptr<IRepository<PBRMaterial>> pbrMaterialRepository, std::shared_ptr<ImageFactory> imageFactory,
    std::shared_ptr<BufferFactory> bufferFactory, std::shared_ptr<BasicGeometryFactory> basicGeometryFactory,
    std::shared_ptr<IRepository<Texture2D>> texture2DRepository)
    : mLogger(mLogger), mContext(context), mPipelineManager(pipelineManager),
      mPipelineLayoutManager(pipelineLayoutManager), mDescriptorManager(descriptorManager),
      mSamplerManager(samplerManager), mImageFactory(imageFactory), mBufferFactory(bufferFactory),
      mBasicGeometryFactory(basicGeometryFactory), mPBRMaterialRepository(pbrMaterialRepository),
      mTexture2DRepository(texture2DRepository)

{
}
entt::entity BasicGeometryEntityManager::CreateCube(std::shared_ptr<entt::registry> registry)
{
    // 1. 创建几何体
    auto geometry = mBasicGeometryFactory->GetGeometry(PrimitiveType::Cube);

    // 2. 创建材质
    std::filesystem::path materialPath = std::filesystem::current_path() / "Resource" / "Material" / "CubeMaterial.mat";
    auto material = mPBRMaterialRepository->Create();
    material->SetRenderType(RenderType::ForwardOpaquePBR);

    mPBRMaterialRepository->Update(material->GetID(), material);
    // 3. 创建网格
    auto mesh = std::make_shared<Mesh>(mBufferFactory, geometry.vertices, geometry.indices);
    // 4. 创建组件对象
    TransformComponent transformComponent;
    transformComponent.position = glm::vec3(0.0f, 0.0f, 0.0f);
    transformComponent.rotation = glm::quat(glm::vec3(1.0f, 1.0f, 1.0f));
    transformComponent.scale = glm::vec3(1.0f, 1.0f, 1.0f);
    // 5. 创建网格组件
    MeshComponent meshComponent;
    meshComponent.mesh = mesh;
    // 6. 创建材质组件
    MaterialComponent materialComponent;
    materialComponent.material = material;

    // 5. 创建实体并添加组件
    entt::entity entity = registry->create();
    registry->emplace<TransformComponent>(entity, std::move(transformComponent));
    registry->emplace<MeshComponent>(entity, std::move(meshComponent));
    registry->emplace<MaterialComponent>(entity, std::move(materialComponent));

    return entity;
}
entt::entity BasicGeometryEntityManager::CreateQuad(std::shared_ptr<entt::registry> registry)
{
    return entt::null;
}
entt::entity BasicGeometryEntityManager::CreateCylinder(std::shared_ptr<entt::registry> registry)
{
    return entt::null;
}
entt::entity BasicGeometryEntityManager::CreateSphere(std::shared_ptr<entt::registry> registry)
{
    return entt::null;
}

} // namespace MEngine