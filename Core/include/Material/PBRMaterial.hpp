#pragma once
#include "Buffer.hpp"
#include "BufferFactory.hpp"
#include "CommandBuffeManager.hpp"
#include "DescriptorManager.hpp"
#include "ImageFactory.hpp"
#include "Interface/IMaterial.hpp"
#include "PipelineManager.hpp"
#include "SyncPrimitiveManager.hpp"
#include "Texture.hpp"
#include "TextureManager.hpp"
#include "glm/fwd.hpp"
#include "magic_enum/magic_enum.hpp"
#include <filesystem>
#include <memory>

namespace MEngine
{

struct PBRParameters
{
    glm::vec3 albedo = {1.0f, 1.0f, 1.0f};
    float metallic = 0.0f;
    float roughness = 0.5f;
    float ao = 1.0f;
    float emissive = 0.0f;
    // TODO: 添加更多参数
};
struct PBRTextureFlag
{
    bool useAlbedoMap = false;
    bool useNormalMap = false;
    bool useMetallicRoughnessMap = false;
    bool useAOMap = false;
    bool useEmissiveMap = false;
};
struct PBRMaterialParams
{
    PBRParameters parameters;
    PBRTextureFlag textureFlag;
};
struct PBRMaterialTextures
{
    std::shared_ptr<Texture> albedoMap = nullptr;
    std::shared_ptr<Texture> normalMap = nullptr;
    std::shared_ptr<Texture> metallicRoughnessMap = nullptr;
    std::shared_ptr<Texture> aoMap = nullptr;
    std::shared_ptr<Texture> emissiveMap = nullptr;
};
class PBRMaterial final : public IMaterial
{
  private:
    // DI
    std::shared_ptr<Context> mContext;
    std::shared_ptr<TextureManager> mTextureManager;
    std::shared_ptr<BufferFactory> mBufferFactory;
    std::shared_ptr<PipelineLayoutManager> mPipelineLayoutManager;
    std::shared_ptr<DescriptorManager> mDescriptorManager;

  private:
    PBRMaterialParams mMaterialParams{};
    PBRMaterialTextures mMaterialTextures{};
    PipelineType mPipelineType = PipelineType::ForwardOpaque;

  private:
    UniqueBuffer mMaterialParamsUBO;
    vk::UniqueDescriptorSet mMaterialDescriptorSet;

  public:
    PBRMaterial(std::shared_ptr<Context> context, std::shared_ptr<TextureManager> textureManager,
                std::shared_ptr<BufferFactory> bufferFactory,
                std::shared_ptr<PipelineLayoutManager> pipelineLayoutManager,
                std::shared_ptr<DescriptorManager> descriptorManager);

  public:
    PipelineType GetPipelineType() const override;
    vk::DescriptorSet GetMaterialDescriptorSet() const override;
    inline const PBRMaterialParams &GetMaterialParams() const
    {
        return mMaterialParams;
    }
    inline const PBRMaterialTextures &GetMaterialTextures() const
    {
        return mMaterialTextures;
    }
    inline void SetPipelineType(PipelineType type) override
    {
        mPipelineType = type;
    }
    void SetPBRParameters(const PBRParameters &params = {});
    void SetPBRTextures(const PBRMaterialTextures &textures = {});
    void SetPBRTextures(std::optional<std::filesystem::path> albedoMapPath = std::nullopt,
                        std::optional<std::filesystem::path> normalMapPath = std::nullopt,
                        std::optional<std::filesystem::path> metallicRoughnessMapPath = std::nullopt,
                        std::optional<std::filesystem::path> aoMapPath = std::nullopt,
                        std::optional<std::filesystem::path> emissiveMapPath = std::nullopt);
    void Update() override;
};
} // namespace MEngine

namespace nlohmann
{
template <> struct adl_serializer<MEngine::PBRMaterial>
{
    static void to_json(json &j, const MEngine::PBRMaterial &m)
    {
        j["pipelineType"] = magic_enum::enum_name(m.GetPipelineType());
        auto materialParams = m.GetMaterialParams();
        j["parameters"]["albedo"] = {materialParams.parameters.albedo.x, materialParams.parameters.albedo.y,
                                     materialParams.parameters.albedo.z};
        j["parameters"]["metallic"] = materialParams.parameters.metallic;
        j["parameters"]["roughness"] = materialParams.parameters.roughness;
        j["parameters"]["ao"] = materialParams.parameters.ao;
        auto materialTextures = m.GetMaterialTextures();
        if (materialTextures.albedoMap != nullptr)
        {
            j["textures"].push_back({{"type", "albedoMap"}, {"path", materialTextures.albedoMap->GetPath().string()}});
        }
        if (materialTextures.normalMap != nullptr)
        {
            j["textures"].push_back({{"type", "normalMap"}, {"path", materialTextures.normalMap->GetPath().string()}});
        }
        if (materialTextures.metallicRoughnessMap != nullptr)
        {
            j["textures"].push_back({{"type", "metallicRoughnessMap"},
                                     {"path", materialTextures.metallicRoughnessMap->GetPath().string()}});
        }
        if (materialTextures.aoMap != nullptr)
        {
            j["textures"].push_back({{"type", "aoMap"}, {"path", materialTextures.aoMap->GetPath().string()}});
        }
        if (materialTextures.emissiveMap != nullptr)
        {
            j["textures"].push_back(
                {{"type", "emissiveMap"}, {"path", materialTextures.emissiveMap->GetPath().string()}});
        }
    }
    static void from_json(const json &j, MEngine::PBRMaterial &m)
    {
        auto pipelineType = j["pipelineType"].get<std::string>();
        m.SetPipelineType(magic_enum::enum_cast<MEngine::PipelineType>(pipelineType).value());
        auto parameters = j["parameters"];
        glm::vec3 albedo(parameters["albedo"][0].get<float>(), parameters["albedo"][1].get<float>(),
                         parameters["albedo"][2].get<float>());
        m.SetPBRParameters({albedo, parameters["metallic"].get<float>(), parameters["roughness"].get<float>(),
                            parameters["ao"].get<float>()});
        auto textures = j["textures"];
        std::optional<std::filesystem::path> albedoMapPath = std::nullopt;
        std::optional<std::filesystem::path> normalMapPath = std::nullopt;
        std::optional<std::filesystem::path> metallicRoughnessMapPath = std::nullopt;
        std::optional<std::filesystem::path> aoMapPath = std::nullopt;
        std::optional<std::filesystem::path> emissiveMapPath = std::nullopt;
        for (const auto &texture : textures)
        {
            auto type = texture["type"].get<std::string>();
            auto path = texture["path"].get<std::string>();
            if (type == "albedoMap")
            {
                albedoMapPath = path;
            }
            else if (type == "normalMap")
            {
                normalMapPath = path;
            }
            else if (type == "metallicRoughnessMap")
            {
                metallicRoughnessMapPath = path;
            }
            else if (type == "aoMap")
            {
                aoMapPath = path;
            }
            else if (type == "emissiveMap")
            {
                emissiveMapPath = path;
            }
            else
            {
                throw std::runtime_error("Unknown texture type: " + type);
            }
        }
        m.SetPBRTextures(albedoMapPath, normalMapPath, metallicRoughnessMapPath, aoMapPath, emissiveMapPath);
        m.Update();
    }
};
} // namespace nlohmann