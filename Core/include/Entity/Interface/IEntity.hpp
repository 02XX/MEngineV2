#pragma once
#include "UUID.hpp"
#include "nlohmann/adl_serializer.hpp"
#include "nlohmann/json.hpp"

namespace MEngine
{
using UUID = UUID;
using UUIDGenerator = UUIDGenerator;
template <typename TKey = UUID> class IEntity
{
  public:
    virtual ~IEntity() = default;
    virtual void SetID(const TKey &id) = 0;
    virtual TKey GetID() const = 0;
    virtual void SetName(const std::string &name) = 0;
    virtual const std::string &GetName() const = 0;
};
} // namespace MEngine

namespace nlohmann
{
template <> struct adl_serializer<MEngine::UUID>
{
    static void to_json(json &j, const MEngine::UUID &uuid)
    {
        j = uuid.ToString();
    }
    static void from_json(const json &j, MEngine::UUID &uuid)
    {
        uuid = MEngine::UUID(j.get<std::string>());
    }
};
template <typename TKey> struct adl_serializer<MEngine::IEntity<TKey>>
{
    static void to_json(json &j, const MEngine::IEntity<TKey> &entity)
    {
        j["id"] = entity.GetID();
        j["name"] = entity.GetName();
    }
    static void from_json(const json &j, MEngine::IEntity<TKey> &entity)
    {
        TKey id = j.at("id").get<TKey>();
        std::string name = j.at("name").get<std::string>();
        entity.SetID(id);
        entity.SetName(name);
    }
};
} // namespace nlohmann