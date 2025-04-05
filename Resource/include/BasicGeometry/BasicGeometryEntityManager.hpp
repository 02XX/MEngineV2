#include "BasicGeometry/BasicGeometryManager.hpp"
#include "BufferManager.hpp"
#include "Componet/MaterialComponent.hpp"
#include "Componet/MeshComponent.hpp"
#include "Componet/TransformComponent.hpp"
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
    std::shared_ptr<entt::registry> mRegistry;
    std::unique_ptr<BasicGeometryManager> mBasicGeometryManager;
    std::shared_ptr<BufferManager> mBufferManager;
    // std::shared_ptr<MaterialManager> mMaterialManager;

  public:
    BasicGeometryEntityManager(std::shared_ptr<entt::registry> registry, std::shared_ptr<BufferManager> bufferManager);
    entt::entity CreateCube();
    entt::entity CreateCylinder();
    entt::entity CreateSphere();
    entt::entity CreateQuad();
};

} // namespace MEngine