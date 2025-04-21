#pragma once
#include "Entity/Interface/IEntity.hpp"
#include "Math.hpp"
#include "magic_enum/magic_enum.hpp"
namespace MEngine
{
enum class LightType
{
    Directional = 0,
    Point = 1,
    Spot = 2,
    Area = 3,
};

class ILight : public virtual IEntity<UUID>
{
    friend class nlohmann::adl_serializer<ILight>;

  public:
    virtual LightType GetLightType() = 0;
    virtual void SetLightType(LightType type) = 0;
    virtual glm::vec3 GetColor() = 0;
    virtual void SetColor(const glm::vec3 &color) = 0;
    virtual float GetIntensity() = 0;
    virtual void SetIntensity(float intensity) = 0;

  public:
    virtual ~ILight() = default;
};
} // namespace MEngine

namespace nlohmann
{
template <> struct adl_serializer<MEngine::ILight>
{
    static void to_json(json &j, const MEngine::ILight &l)
    {
        auto &entity = static_cast<const MEngine::IEntity<MEngine::UUID> &>(l);
        j = entity;
    }
    static void from_json(const json &j, MEngine::ILight &l)
    {
        auto &entity = static_cast<MEngine::IEntity<MEngine::UUID> &>(l);
        j.get_to(entity);
    }
};
} // namespace nlohmann