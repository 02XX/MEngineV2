
#include "MEngine.hpp"
#include "Mesh.hpp"
#include "NoCopyable.hpp"
#include "Vertex.hpp"
#include "glm/glm.hpp"
#include <cstdint>
#include <glm/gtc/constants.hpp>
#include <unordered_map>
#include <vector>

namespace MEngine
{
enum class PrimitiveType
{
    Cube,     // 立方体
    Cylinder, // 圆柱体
    Sphere,   // 球体
    Quad,     // 四边形
};
struct Geometry
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};
class BasicGeometryFactory : public NoCopyable
{
  public:
    BasicGeometryFactory();

    Geometry GetGeometry(PrimitiveType type);

  private:
    Geometry GetCube();
    Geometry GetCylinder();
    Geometry GetSphere();
    Geometry GetQuad();
};

} // namespace MEngine