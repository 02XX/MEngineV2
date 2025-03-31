#include "BasicGeometry/BasicGeometryManager.hpp"
#include "Componet/MaterialComponent.hpp"
#include "Componet/MeshComponent.hpp"
#include "Componet/TransformComponent.hpp"
#include "Logger.hpp"
#include "MEngine.hpp"
// #include "MaterialManager.hpp"
#include "NoCopyable.hpp"
#include "PipelineLayoutManager.hpp"
#include "PipelineManager.hpp"
#include "entt/entt.hpp"

namespace MEngine
{
class BasicGeometryEntityManager : public NoCopyable
{
  private:
    std::shared_ptr<entt::registry> mRegistry;
    std::unique_ptr<BasicGeometryManager> mBasicGeometryManager;
    // std::shared_ptr<MaterialManager> mMaterialManager;

  public:
    BasicGeometryEntityManager(std::shared_ptr<entt::registry> registry);
    entt::entity CreateCube();
    entt::entity CreateCylinder();
    entt::entity CreateSphere();
    entt::entity CreateQuad();
};

} // namespace MEngine