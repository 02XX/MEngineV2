#pragma once
#include "Entity/Interface/IEntity.hpp"

namespace MEngine
{
template <typename TKey = UUID> class Entity : public virtual IEntity<TKey>
{
  protected:
    TKey mID = UUIDGenerator()();
    std::string mName = "DefaultEntity";

  public:
    Entity() = default;
    virtual ~Entity() = default;

  public:
    inline virtual void SetID(const TKey &id) override
    {
        mID = id;
    }
    inline virtual TKey GetID() const override
    {
        return mID;
    }
    inline virtual void SetName(const std::string &name) override
    {
        mName = name;
    }
    inline virtual const std::string &GetName() const override
    {
        return mName;
    }
};
} // namespace MEngine