#include "BasicGeometry/BasicGeometryManager.hpp"
#include "BufferManager.hpp"
#include "Componet/MaterialComponent.hpp"
#include "Componet/MeshComponent.hpp"
#include "Componet/TransformComponent.hpp"
#include "Context.hpp"
#include "MEngine.hpp"
// #include "MaterialManager.hpp"
#include "NoCopyable.hpp"
#include "PipelineLayoutManager.hpp"
#include "PipelineManager.hpp"
#include "entt/entt.hpp"
#include <memory>

namespace MEngine
{
class BasicGeometryEntityManager : public NoCopyable
{
  private:
    // DI
    std::unique_ptr<BasicGeometryManager> mBasicGeometryManager;

    std::shared_ptr<Context> mContext;
    std::shared_ptr<ILogger> mLogger;
    std::shared_ptr<BufferManager> mBufferManager;
    std::shared_ptr<PipelineManager> mPipelineManager;
    std::shared_ptr<PipelineLayoutManager> mPipelineLayoutManager;
    std::shared_ptr<DescriptorManager> mDescriptorManager;

  public:
    BasicGeometryEntityManager(std::shared_ptr<Context> context, std::shared_ptr<ILogger> logger,
                               std::shared_ptr<BufferManager> bufferManager,
                               std::shared_ptr<PipelineManager> pipelineManager,
                               std::shared_ptr<PipelineLayoutManager> pipelineLayoutManager,
                               std::shared_ptr<DescriptorManager> descriptorManager);
    entt::entity CreateCube(std::shared_ptr<entt::registry> registry);
    entt::entity CreateCylinder(std::shared_ptr<entt::registry> registry);
    entt::entity CreateSphere(std::shared_ptr<entt::registry> registry);
    entt::entity CreateQuad(std::shared_ptr<entt::registry> registry);
};

} // namespace MEngine