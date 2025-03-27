#include "BasicGeometry/BasicGeometryManager.hpp"
#include "Componet/MaterialComponent.hpp"
#include "Componet/MeshComponent.hpp"
#include "Componet/TransformComponent.hpp"
#include "Logger.hpp"
#include "MEngine.hpp"
#include "NoCopyable.hpp"
#include "entt/entt.hpp"
#include <memory>


namespace MEngine
{
class MENGINE_API BasicGeometryEntityManager : public NoCopyable
{
  private:
    std::shared_ptr<entt::registry> mRegistry;
    std::unique_ptr<BasicGeometryManager> mBasicGeometryManager;

  public:
    BasicGeometryEntityManager() = default;
    entt::entity CreateCube();
    entt::entity CreateCylinder();
    entt::entity CreateSphere();
    entt::entity CreateQuad();
};

} // namespace MEngine