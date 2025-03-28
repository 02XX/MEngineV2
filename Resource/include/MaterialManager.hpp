#include "MEngine.hpp"
#include "Material.hpp"
#include "NoCopyable.hpp"
#include <unordered_map>
namespace MEngine
{
enum class MaterialType
{
    Default,
    Custom,
};
class MENGINE_API MaterialManager : public NoCopyable
{
  private:
    std::unordered_map<MaterialType, std::weak_ptr<Material>> mMaterials;

    std::shared_ptr<Material> CreateDefaultMaterial();

  public:
    MaterialManager() = default;

    std::shared_ptr<Material> GetMaterial(MaterialType type);
};
} // namespace MEngine