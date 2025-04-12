#include "Context.hpp"
#include "DescriptorManager.hpp"
#include "MEngine.hpp"
#include "Material.hpp"
#include "NoCopyable.hpp"
#include "PipelineManager.hpp"
#include "SamplerManager.hpp"
#include "TextureManager.hpp"
#include "stb_image.h"
#include <cstdint>
#include <filesystem>
#include <memory>
#include <unordered_map>
namespace MEngine
{
class MaterialManager : public NoCopyable
{
  private:
    // DI
    std::shared_ptr<ILogger> mLogger;
    std::shared_ptr<Context> mContext;
    std::shared_ptr<PipelineManager> mPipelineManager;
    std::shared_ptr<PipelineLayoutManager> mPipelineLayoutManager;
    std::shared_ptr<DescriptorManager> mDescriptorManager;
    std::shared_ptr<SamplerManager> mSamplerManager;
    std::shared_ptr<TextureManager> mTextureManager;

  private:
    //{Id, Material}
    std::unordered_map<uint32_t, std::weak_ptr<Material>> mMaterials; // 材质
  private:
    std::filesystem::path mDefaultMaterialPath = std::filesystem::current_path() / "Resource" / "Material";
    std::filesystem::path mDefaultAlbedoPath = mDefaultMaterialPath / "DefaultAlbedo.png";

  public:
    MaterialManager(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                    std::shared_ptr<PipelineManager> pipelineManager,
                    std::shared_ptr<PipelineLayoutManager> pipelineLayoutManager,
                    std::shared_ptr<DescriptorManager> descriptorManager,
                    std::shared_ptr<SamplerManager> samplerManager, std::shared_ptr<TextureManager> textureManager);
    std::shared_ptr<Material> CreateTranslucencyMaterial(std::filesystem::path baseColorPath);
    std::shared_ptr<Material> GetMaterial(uint32_t id);
    std::shared_ptr<Material> GetDefaultMaterial();
};
} // namespace MEngine