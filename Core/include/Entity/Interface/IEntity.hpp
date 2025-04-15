#pragma once
#include "NoCopyable.hpp"
#include "boost/uuid.hpp"
namespace MEngine
{
using UUID = boost::uuids::uuid;
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