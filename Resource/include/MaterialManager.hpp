#include "Context.hpp"
#include "DescriptorManager.hpp"
#include "ImageManager.hpp"
#include "MEngine.hpp"
#include "Material.hpp"
#include "NoCopyable.hpp"
#include "SamplerManager.hpp"
#include "stb_image.h"
#include <memory>
#include <unordered_map>

namespace MEngine
{
enum class MaterialType
{
    Default,
    PBR,
    Custom,
};
class MaterialManager : public NoCopyable
{
  private:
    std::unordered_map<MaterialType, std::weak_ptr<Material>> mMaterials;
    std::unique_ptr<DescriptorManager> mDescriptorManager;
    std::shared_ptr<Material> CreateDefaultMaterial();
    std::shared_ptr<Material> CreatePBRMaterial();
    std::unique_ptr<ImageManager> mImageManager;
    std::unique_ptr<SamplerManagerManager> mSamplerManager;

  public:
    MaterialManager();

    std::shared_ptr<Material> GetMaterial(MaterialType type);
};
} // namespace MEngine