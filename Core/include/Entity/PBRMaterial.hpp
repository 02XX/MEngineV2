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
#include <utility>
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
    alignas(4) bool useAlbedoMap = false;
    alignas(4) bool useNormalMap = false;
    alignas(4) bool useMetallicRoughnessMap = false;
    alignas(4) bool useAOMap = false;
    alignas(4) bool useEmissiveMap = false;
};
struct PBRParams
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
class PBRMaterialMetadata : public IMaterialMetadata
{
  public:
    glm::vec3 albedo = {1.0f, 1.0f, 1.0f};
    float metallic = 0.0f;
    float roughness = 0.5f;
    float ao = 1.0f;
    float emissive = 0.0f;
    // 外键
    UUID AlbedoMapID{};
    UUID NormalMapID{};
    UUID MetallicRoughnessMapID{};
    UUID AOMapID{};
    UUID EmissiveMapID{};
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
    std::string mMaterialName = "PBRMaterial";
    uint32_t mMaterialID = 0;
    std::filesystem::path mMaterialPath;
    PBRParams mMaterialParams{};
    PBRMaterialTextures mMaterialTextures{};
    PipelineType mPipelineType = PipelineType::ForwardOpaque;

  private:
    UniqueBuffer mMaterialParamsUBO;
    vk::UniqueDescriptorSet mMaterialDescriptorSet;

  public:
    PBRMaterial(std::shared_ptr<Context> context, std::shared_ptr<TextureManager> textureManager,
                std::shared_ptr<BufferFactory> bufferFactory,
                std::shared_ptr<PipelineLayoutManager> pipelineLayoutManager,
                std::shared_ptr<DescriptorManager> descriptorManager)
        : mContext(context), mTextureManager(textureManager), mBufferFactory(bufferFactory),
          mPipelineLayoutManager(pipelineLayoutManager), mDescriptorManager(descriptorManager)
    {
    }
    ~PBRMaterial() override = default;
};
} // namespace MEngine

namespace nlohmann
{
template <> struct adl_serializer<MEngine::PBRMaterialMetadata>
{
    static void to_json(json &j, const MEngine::PBRMaterialMetadata &m)
    {
        auto pipelineTypeStr = magic_enum::enum_name(m.pipelineType);
        j["pipelineType"] = pipelineTypeStr;
        j["parameters"]["albedo"] = {m.albedo.x, m.albedo.y, m.albedo.z};
        j["parameters"]["metallic"] = m.metallic;
        j["parameters"]["roughness"] = m.roughness;
        j["parameters"]["ao"] = m.ao;
        j["textures"].push_back(
            {{"type", "albedoMap"}, {"path", m.albedoMapPath.has_value() ? m.albedoMapPath.value() : ""}});
        j["textures"].push_back(
            {{"type", "normalMap"}, {"path", m.normalMapPath.has_value() ? m.normalMapPath.value() : ""}});
        j["textures"].push_back(
            {{"type", "metallicRoughnessMap"},
             {"path", m.metallicRoughnessMapPath.has_value() ? m.metallicRoughnessMapPath.value() : ""}});
        j["textures"].push_back({{"type", "aoMap"}, {"path", m.aoMapPath.has_value() ? m.aoMapPath.value() : ""}});
        j["textures"].push_back(
            {{"type", "emissiveMap"}, {"path", m.emissiveMapPath.has_value() ? m.emissiveMapPath.value() : ""}});
    }
    static void from_json(const json &j, MEngine::PBRMaterialMetadata &m)
    {
        auto pipelineTypeStr = j["pipelineType"].get<std::string>();
        auto pipelineTypeOpt = magic_enum::enum_cast<MEngine::PipelineType>(pipelineTypeStr);
        if (pipelineTypeOpt.has_value())
        {
            m.pipelineType = pipelineTypeOpt.value();
        }
        else
        {
            throw std::runtime_error("Invalid pipeline type: " + pipelineTypeStr);
        }
        m.albedo.x = j["parameters"]["albedo"][0].get<float>();
        m.albedo.y = j["parameters"]["albedo"][1].get<float>();
        m.albedo.z = j["parameters"]["albedo"][2].get<float>();
        m.metallic = j["parameters"]["metallic"].get<float>();
        m.roughness = j["parameters"]["roughness"].get<float>();
        m.ao = j["parameters"]["ao"].get<float>();
        std::pair<std::string, std::string> texture;
        for (const auto &textureJson : j["textures"])
        {
            texture.first = textureJson["type"].get<std::string>();
            texture.second = textureJson["path"].get<std::string>();
            if (texture.first == "albedoMap")
                m.albedoMapPath =
                    texture.second == "" ? std::nullopt : std::optional<std::filesystem::path>(texture.second);
            else if (texture.first == "normalMap")
                m.normalMapPath =
                    texture.second == "" ? std::nullopt : std::optional<std::filesystem::path>(texture.second);
            else if (texture.first == "metallicRoughnessMap")
                m.metallicRoughnessMapPath =
                    texture.second == "" ? std::nullopt : std::optional<std::filesystem::path>(texture.second);
            else if (texture.first == "aoMap")
                m.aoMapPath =
                    texture.second == "" ? std::nullopt : std::optional<std::filesystem::path>(texture.second);
            else if (texture.first == "emissiveMap")
                m.emissiveMapPath =
                    texture.second == "" ? std::nullopt : std::optional<std::filesystem::path>(texture.second);
        }
    }
};
} // namespace nlohmann