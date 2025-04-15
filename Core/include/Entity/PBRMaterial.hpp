#pragma once
#include "Buffer.hpp"
#include "BufferFactory.hpp"
#include "CommandBuffeManager.hpp"
#include "DescriptorManager.hpp"
#include "Entity.hpp"
#include "Entity/Entity.hpp"
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
    float roughness = 0.5F;
    float ao = 1.0f;
    float emissive = 0.0f;
    // TODO: 添加更多参数
};
struct PBRTextureFlag
{
    uint32_t useAlbedoMap = 0; // 与GLSL的bool字节一致 4字节
    uint32_t useNormalMap = 0;
    uint32_t useMetallicRoughnessMap = 0;
    uint32_t useAOMap = 0;
    uint32_t useEmissiveMap = 0;
};
struct PBRParams
{
    PBRParameters parameters;
    PBRTextureFlag textureFlag;
};
class PBRMaterial final : public IMaterial<>, public Entity<>
{
  private:
    PBRParams mMaterialParams;
    // 外键
    UUID AlbedoMapID{};
    UUID NormalMapID{};
    UUID MetallicRoughnessMapID{};
    UUID AOMapID{};
    UUID EmissiveMapID{};

  private:
  private:
    // Vulkan Resources
    UniqueBuffer mMaterialParamsUBO;
    vk::UniqueDescriptorSet mMaterialDescriptorSet;

  public:
    // Getters
    RenderType GetRenderType() const override;
    inline const PBRParams &GetMaterialParams() const
    {
        return mMaterialParams;
    }
    inline const UUID &GetAlbedoMapID() const
    {
        return AlbedoMapID;
    }
    inline const UUID &GetNormalMapID() const
    {
        return NormalMapID;
    }
    inline const UUID &GetMetallicRoughnessMapID() const
    {
        return MetallicRoughnessMapID;
    }
    inline const UUID &GetAOMapID() const
    {
        return AOMapID;
    }
    inline const UUID &GetEmissiveMapID() const
    {
        return EmissiveMapID;
    }
    inline void SetMaterialParams(const PBRParams &params)
    {
        mMaterialParams = params;
    }
    inline void SetAlbedoMapID(const UUID &id)
    {
        AlbedoMapID = id;
    }
    inline void SetNormalMapID(const UUID &id)
    {
        NormalMapID = id;
    }
    inline void SetMetallicRoughnessMapID(const UUID &id)
    {
        MetallicRoughnessMapID = id;
    }
    inline void SetAOMapID(const UUID &id)
    {
        AOMapID = id;
    }
    inline void SetEmissiveMapID(const UUID &id)
    {
        EmissiveMapID = id;
    }
    // Setters
    void SetRenderType(RenderType type) override;
    // Vulkan Resources
    vk::DescriptorSet GetDescriptorSet() const override;
};
} // namespace MEngine

namespace nlohmann
{
template <> struct adl_serializer<MEngine::PBRMaterial>
{
    static void to_json(json &j, const MEngine::PBRMaterial &m)
    {
        auto renderType = magic_enum::enum_name(m.GetRenderType());
        j["RenderType"] = renderType;
        j["Albedo"] = {m.GetMaterialParams().parameters.albedo.x, m.GetMaterialParams().parameters.albedo.y,
                       m.GetMaterialParams().parameters.albedo.z};
        j["Metallic"] = m.GetMaterialParams().parameters.metallic;
        j["Roughness"] = m.GetMaterialParams().parameters.roughness;
        j["AO"] = m.GetMaterialParams().parameters.ao;
        j["Emissive"] = m.GetMaterialParams().parameters.emissive;
        j["Textures"].push_back({"AlbedoMap", m.GetAlbedoMapID()});
        j["Textures"].push_back({"NormalMap", m.GetNormalMapID()});
        j["Textures"].push_back({"MetallicRoughnessMap", m.GetMetallicRoughnessMapID()});
        j["Textures"].push_back({"AOMap", m.GetAOMapID()});
        j["Textures"].push_back({"EmissiveMap", m.GetEmissiveMapID()});
    }
    static void from_json(const json &j, MEngine::PBRMaterial &m)
    {
        auto renderType = j.at("RenderType").get<std::string>();
        m.SetRenderType(magic_enum::enum_cast<MEngine::RenderType>(renderType).value());
        auto albedoArray = j.at("Albedo").get<std::array<float, 3>>();
        glm::vec3 albedo(albedoArray[0], albedoArray[1], albedoArray[2]);
        m.SetMaterialParams({albedo, j.at("Metallic").get<float>(), j.at("Roughness").get<float>(),
                             j.at("AO").get<float>(), j.at("Emissive").get<float>()});
        auto textures = j.at("Textures");
        for (const auto &texture : textures)
        {
            auto type = texture.at("Type").get<std::string>();
            auto textureID = texture.at("ID").get<MEngine::UUID>();
            if (type == "AlbedoMap")
            {
                m.SetAlbedoMapID(textureID);
            }
            else if (type == "NormalMap")
            {
                m.SetNormalMapID(textureID);
            }
            else if (type == "MetallicRoughnessMap")
            {
                m.SetMetallicRoughnessMapID(textureID);
            }
            else if (type == "AOMap")
            {
                m.SetAOMapID(textureID);
            }
            else if (type == "EmissiveMap")
            {
                m.SetEmissiveMapID(textureID);
            }
        }
    }
};
} // namespace nlohmann
