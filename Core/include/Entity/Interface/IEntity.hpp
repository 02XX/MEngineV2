#pragma once
#include "NoCopyable.hpp"
#include "boost/uuid.hpp"
#include "nlohmann/adl_serializer.hpp"
#include "nlohmann/json.hpp"
namespace MEngine
{
using UUID = boost::uuids::uuid;
using UUIDGenerator = boost::uuids::random_generator;
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
        j = boost::uuids::to_string(uuid);
    }
    static void from_json(const json &j, MEngine::UUID &uuid)
    {
        boost::uuids::string_generator gen;
        uuid = gen(j.get<std::string>());
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