#include "BasicGeometry/BasicGeometryFactory.hpp"
#include "BufferFactory.hpp"
#include "Component/MaterialComponent.hpp"
#include "Component/MeshComponent.hpp"
#include "Component/TransformComponent.hpp"
#include "Context.hpp"
#include "DescriptorManager.hpp"
#include "Entity/Interface/IMaterial.hpp"
#include "Entity/Interface/ITexture.hpp"
#include "Entity/PBRMaterial.hpp"
#include "Entity/Texture2D.hpp"
#include "NoCopyable.hpp"
#include "PipelineLayoutManager.hpp"
#include "PipelineManager.hpp"
#include "Repository/Interface/IRepository.hpp"
#include "SamplerManager.hpp"
#include "entt/entt.hpp"
#include <memory>

namespace MEngine
{
class BasicGeometryEntityManager : public NoCopyable
{
  private:
    // DI
    std::shared_ptr<ILogger> mLogger;
    std::shared_ptr<Context> mContext;
    std::shared_ptr<PipelineManager> mPipelineManager;
    std::shared_ptr<PipelineLayoutManager> mPipelineLayoutManager;
    std::shared_ptr<DescriptorManager> mDescriptorManager;
    std::shared_ptr<SamplerManager> mSamplerManager;
    std::shared_ptr<IRepository<PBRMaterial>> mPBRMaterialRepository;

    std::shared_ptr<ImageFactory> mImageFactory;
    std::shared_ptr<BufferFactory> mBufferFactory;
    std::shared_ptr<BasicGeometryFactory> mBasicGeometryFactory;
    std::shared_ptr<IRepository<Texture2D>> mTexture2DRepository;

  private:
  public:
    BasicGeometryEntityManager(std::shared_ptr<ILogger> mLogger, std::shared_ptr<Context> context,
                               std::shared_ptr<PipelineManager> pipelineManager,
                               std::shared_ptr<PipelineLayoutManager> pipelineLayoutManager,
                               std::shared_ptr<DescriptorManager> descriptorManager,
                               std::shared_ptr<SamplerManager> samplerManager,
                               std::shared_ptr<IRepository<PBRMaterial>> pbrMaterialRepository,
                               std::shared_ptr<ImageFactory> imageFactory, std::shared_ptr<BufferFactory> bufferFactory,
                               std::shared_ptr<BasicGeometryFactory> basicGeometryFactory,
                               std::shared_ptr<IRepository<Texture2D>> texture2DRepository);
    entt::entity CreateCube(std::shared_ptr<entt::registry> registry);
    entt::entity CreateCylinder(std::shared_ptr<entt::registry> registry);
    entt::entity CreateSphere(std::shared_ptr<entt::registry> registry);
    entt::entity CreateQuad(std::shared_ptr<entt::registry> registry);
};

} // namespace MEngine