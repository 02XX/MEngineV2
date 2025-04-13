#include "Configure.hpp"
#include "Context.hpp"
#include "DescriptorManager.hpp"
#include "MEngine.hpp"
#include "Material.hpp"
#include "NoCopyable.hpp"
#include "PipelineLayoutManager.hpp"
#include "PipelineManager.hpp"
#include "SamplerManager.hpp"
#include "Texture.hpp"
#include "TextureManager.hpp"
#include "magic_enum/magic_enum.hpp"
#include "nlohmann/json.hpp"
#include "stb_image.h"
#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

namespace MEngine
{
struct MaterialParams
{
    PipelineLayoutType pipelineLayoutType;
    PipelineType pipelineType;
    std::unordered_map<TextureType, std::filesystem::path> texturePaths;
};
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
    std::unordered_map<uint32_t, std::shared_ptr<Material>> mMaterials; // 材质
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
    std::shared_ptr<Material> GetDefaultMaterial();
    std::shared_ptr<Material> GetMaterial(uint32_t id);
    uint32_t LoadMaterial(std::filesystem::path materialPath);
    void SaveMaterial(std::filesystem::path materialPath, std::shared_ptr<Material> material);
    void CreateMaterial(std::filesystem::path materialPath);
};
} // namespace MEngine

namespace nlohmann
{
template <> struct adl_serializer<MEngine::MaterialParams>
{
    static void to_json(json &j, const MEngine::MaterialParams &m)
    {
        j["pipelineLayoutType"] = m.pipelineLayoutType;
        j["pipelineType"] = m.pipelineType;
        for (const auto &texture : m.texturePaths)
        {
            j["textures"].push_back({{"type", magic_enum::enum_name(texture.first)}, {"path", texture.second}});
        }
    }
    static void from_json(const json &j, MEngine::MaterialParams &m)
    {
        j.at("pipelineLayoutType").get_to(m.pipelineLayoutType);
        j.at("pipelineType").get_to(m.pipelineType);
        for (const auto &texture : j.at("textures"))
        {
            MEngine::TextureType type = texture.at("type").get<MEngine::TextureType>();
            std::filesystem::path path = texture.at("path").get<std::string>();
            m.texturePaths[magic_enum::enum_cast<MEngine::TextureType>(texture.at("type").get<std::string>()).value()] =
                path;
        }
    }
};
} // namespace nlohmann