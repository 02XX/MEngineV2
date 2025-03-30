#pragma once
#include "MEngine.hpp"
namespace MEngine
{
class ISystem
{
  public:
    ISystem() = default;
    virtual ~ISystem() = default;

    virtual void Init() = 0;
    virtual void Tick(float deltaTime) = 0;
    virtual void Shutdown() = 0;
};
} // namespace MEngine