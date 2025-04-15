#pragma once
#include "Entity/Interface/IEntity.hpp"

namespace MEngine
{
class Entity : public IEntity
{
  private:
    uint32_t mID{};
    std::string mName = "DefaultEntity";

  public:
    Entity() = default;
    Entity(uint32_t id, const std::string &name) : mID(id), mName(name) {};
    ~Entity() override = default;

  public:
    inline void SetID(uint32_t id) override
    {
        mID = id;
    }
    inline uint32_t GetID() const override
    {
        return mID;
    }
    inline void SetName(const std::string &name) override
    {
        mName = name;
    }
    inline const std::string &GetName() const override
    {
        return mName;
    }
};
} // namespace MEngine