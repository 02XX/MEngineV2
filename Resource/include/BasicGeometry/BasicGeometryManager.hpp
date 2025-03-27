#include "Logger.hpp"
#include "MEngine.hpp"
#include "Mesh.hpp"
#include "NoCopyable.hpp"
#include "glm/glm.hpp"
#include <glm/gtc/constants.hpp>
#include <unordered_map>

namespace MEngine
{
enum class PrimitiveType
{
    Cube,     // 立方体
    Cylinder, // 圆柱体
    Sphere,   // 球体
    Quad,     // 四边形
};
class MENGINE_API BasicGeometryManager : public NoCopyable
{
  private:
    std::unordered_map<PrimitiveType, std::weak_ptr<Mesh>> mCache;
    std::shared_ptr<Mesh> CreateCube();
    std::shared_ptr<Mesh> CreateCylinder();
    std::shared_ptr<Mesh> CreateSphere();
    std::shared_ptr<Mesh> CreateQuad();

  public:
    BasicGeometryManager() = default;
    std::shared_ptr<Mesh> GetPrimitive(PrimitiveType type);
};

} // namespace MEngine