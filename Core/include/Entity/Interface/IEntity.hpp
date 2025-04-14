#pragma once

#include "NoCopyable.hpp"

namespace MEngine
{
class IEntity : public NoCopyable
{
  public:
    virtual ~IEntity() = default;
    virtual void SetID(uint32_t id) = 0;
    virtual uint32_t GetID() const = 0;
    virtual void SetName(const std::string &name) = 0;
    virtual const std::string &GetName() const = 0;
};
} // namespace MEngine