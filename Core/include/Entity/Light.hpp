#pragma once
#include "Entity.hpp"
#include "Entity/Entity.hpp"
#include "Interface/IEntity.hpp"
#include "Interface/ILight.hpp"
#include "Math.hpp"
#include "magic_enum/magic_enum.hpp"
#include "nlohmann/adl_serializer.hpp"

namespace MEngine
{
class Light : public ILight, public Entity<>
{
  public:
    LightType type = LightType::Directional;
    glm::vec3 color = {1.0f, 1.0f, 1.0f}; // 颜色
    float intensity = 1.0f;               // 强度
  public:
    Light(LightType type = LightType::Directional, const glm::vec3 &color = {1.0f, 1.0f, 1.0f}, float intensity = 1.0f)
        : type(type), color(color), intensity(intensity)
    {
    }
    virtual ~Light() = default;
};
} // namespace MEngine

namespace nlohmann
{
template <> struct adl_serializer<MEngine::Light>
{
    static void to_json(json &j, const MEngine::Light &l)
    {
        auto &entity = static_cast<const MEngine::IEntity<MEngine::UUID> &>(l);
        j = entity;
        j["Type"] = magic_enum::enum_name(l.type);
        j["Color"] = {l.color.r, l.color.g, l.color.b};
        j["Intensity"] = l.intensity;
    }
    static void from_json(const json &j, MEngine::Light &l)
    {
        auto &entity = static_cast<MEngine::IEntity<MEngine::UUID> &>(l);
        j.get_to(entity);
        auto type = j.at("Type").get<std::string>();
        l.type = magic_enum::enum_cast<MEngine::LightType>(type).value();
        auto colorArray = j.at("Color").get<std::array<float, 3>>();
        l.color = glm::vec3(colorArray[0], colorArray[1], colorArray[2]);
        l.intensity = j.at("Intensity").get<float>();
    }
};
} // namespace nlohmann