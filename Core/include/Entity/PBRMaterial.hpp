#pragma once
#include "Buffer.hpp"
#include "Entity.hpp"
#include "Entity/Entity.hpp"
#include "Interface/IEntity.hpp"
#include "Interface/IMaterial.hpp"
#include "Math.hpp"
#include "magic_enum/magic_enum.hpp"
#include "nlohmann/adl_serializer.hpp"
namespace MEngine
{

struct PBRParameters
{
    alignas(16) glm::vec3 albedo = {1.0f, 1.0f, 1.0f};
    float metallic = 0.5f;
    float roughness = 0.04f;
    float ao = 1.0f;
    float emissive = 0.0f;
    // TODO: 添加更多参数
};
struct PBRTextureFlag
{
    uint32_t useAlbedoMap = 0; // 4字节
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
class PBRMaterial final : public IMaterial, public Entity<>
{
    friend nlohmann::adl_serializer<MEngine::PBRMaterial>;
    friend class PBRMaterialRepository;

  private:
    RenderType mRenderType = RenderType::ForwardOpaquePBR;
    PBRParams mMaterialParams;
    // 外键
    UUID mAlbedoMapID{};
    UUID mNormalMapID{};
    UUID mMetallicRoughnessMapID{};
    UUID mAOMapID{};
    UUID mEmissiveMapID{};

  private:
    // Vulkan Resources
    UniqueBuffer mMaterialParamsUBO;
    vk::UniqueDescriptorSet mMaterialDescriptorSet;

  public:
    PBRMaterial();
    ~PBRMaterial() = default;
    // Getters
    inline RenderType GetRenderType() const override
    {
        return mRenderType;
    }
    inline const PBRParams &GetMaterialParams() const
    {
        return mMaterialParams;
    }
    inline const UUID &GetAlbedoMapID() const
    {
        return mAlbedoMapID;
    }
    inline const UUID &GetNormalMapID() const
    {
        return mNormalMapID;
    }
    inline const UUID &GetMetallicRoughnessMapID() const
    {
        return mMetallicRoughnessMapID;
    }
    inline const UUID &GetAOMapID() const
    {
        return mAOMapID;
    }
    inline const UUID &GetEmissiveMapID() const
    {
        return mEmissiveMapID;
    }
    inline void SetRenderType(RenderType renderType) override
    {
        mRenderType = renderType;
    }
    inline void SetMaterialParams(const PBRParams &params)
    {
        mMaterialParams = params;
    }
    inline void SetAlbedoMapID(const UUID &id)
    {
        mAlbedoMapID = id;
        if (mAlbedoMapID.IsEmpty())
        {
            mMaterialParams.textureFlag.useAlbedoMap = 0;
        }
        else
        {
            mMaterialParams.textureFlag.useAlbedoMap = 1;
        }
    }
    inline void SetNormalMapID(const UUID &id)
    {
        mNormalMapID = id;
        if (mNormalMapID.IsEmpty())
        {
            mMaterialParams.textureFlag.useNormalMap = 0;
        }
        else
        {
            mMaterialParams.textureFlag.useNormalMap = 1;
        }
    }
    inline void SetMetallicRoughnessMapID(const UUID &id)
    {
        mMetallicRoughnessMapID = id;
        if (mMetallicRoughnessMapID.IsEmpty())
        {
            mMaterialParams.textureFlag.useMetallicRoughnessMap = 0;
        }
        else
        {
            mMaterialParams.textureFlag.useMetallicRoughnessMap = 1;
        }
    }
    inline void SetAOMapID(const UUID &id)
    {
        mAOMapID = id;
        if (mAOMapID.IsEmpty())
        {
            mMaterialParams.textureFlag.useAOMap = 0;
        }
        else
        {
            mMaterialParams.textureFlag.useAOMap = 1;
        }
    }
    inline void SetEmissiveMapID(const UUID &id)
    {
        mEmissiveMapID = id;
        if (mEmissiveMapID.IsEmpty())
        {
            mMaterialParams.textureFlag.useEmissiveMap = 0;
        }
        else
        {
            mMaterialParams.textureFlag.useEmissiveMap = 1;
        }
    }
    // Vulkan Resources
    vk::DescriptorSet GetDescriptorSet() const override
    {
        return mMaterialDescriptorSet.get();
    }
};
} // namespace MEngine

namespace nlohmann
{
template <> struct adl_serializer<MEngine::PBRMaterial>
{
    static void to_json(json &j, const MEngine::PBRMaterial &m)
    {
        auto &material = static_cast<const MEngine::IMaterial &>(m);
        j = material;
        auto renderType = magic_enum::enum_name(m.GetRenderType());
        j["RenderType"] = renderType;
        j["Textures"] = nlohmann::json::object();
        j["Textures"]["AlbedoMap"] = m.GetAlbedoMapID();
        j["Textures"]["NormalMap"] = m.GetNormalMapID();
        j["Textures"]["MetallicRoughnessMap"] = m.GetMetallicRoughnessMapID();
        j["Textures"]["AOMap"] = m.GetAOMapID();
        j["Textures"]["EmissiveMap"] = m.GetEmissiveMapID();
    }
    static void from_json(const json &j, MEngine::PBRMaterial &m)
    {
        auto &material = static_cast<MEngine::IMaterial &>(m);
        j.get_to(material);
        auto renderType = j.at("RenderType").get<std::string>();
        m.mRenderType = magic_enum::enum_cast<MEngine::RenderType>(renderType).value();
        auto albedoArray = j.at("Albedo").get<std::array<float, 3>>();
        glm::vec3 albedo(albedoArray[0], albedoArray[1], albedoArray[2]);
        m.mMaterialParams.parameters.albedo = albedo;
        m.mMaterialParams.parameters.metallic = j.at("Metallic").get<float>();
        m.mMaterialParams.parameters.roughness = j.at("Roughness").get<float>();
        m.mMaterialParams.parameters.ao = j.at("AO").get<float>();
        m.mMaterialParams.parameters.emissive = j.at("Emissive").get<float>();
        auto textures = j.at("Textures");
        for (const auto &texture : textures)
        {
            auto type = texture.at("Type").get<std::string>();
            auto textureID = texture.at("ID").get<MEngine::UUID>();
            if (type == "AlbedoMap")
            {
                auto textureID = texture.at("ID").get<MEngine::UUID>();
                if (m.mAlbedoMapID.IsEmpty())
                {
                    m.mMaterialParams.textureFlag.useAlbedoMap = 1;
                }
                m.mAlbedoMapID = textureID;
            }
            else if (type == "NormalMap")
            {
                if (m.mNormalMapID.IsEmpty())
                {
                    m.mMaterialParams.textureFlag.useNormalMap = 1;
                }
                m.mNormalMapID = textureID;
            }
            else if (type == "MetallicRoughnessMap")
            {
                if (m.mMetallicRoughnessMapID.IsEmpty())
                {
                    m.mMaterialParams.textureFlag.useMetallicRoughnessMap = 1;
                }
                m.mMetallicRoughnessMapID = textureID;
            }
            else if (type == "AOMap")
            {
                if (m.mAOMapID.IsEmpty())
                {
                    m.mMaterialParams.textureFlag.useAOMap = 1;
                }
                m.mAOMapID = textureID;
            }
            else if (type == "EmissiveMap")
            {
                if (m.mEmissiveMapID.IsEmpty())
                {
                    m.mMaterialParams.textureFlag.useEmissiveMap = 1;
                }
                m.mEmissiveMapID = textureID;
            }
            else
            {
                throw std::runtime_error("Unknown texture type: " + type);
            }
        }
    }
};
} // namespace nlohmann
