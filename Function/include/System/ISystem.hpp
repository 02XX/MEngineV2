#pragma once
#include "Component/Interface/IComponent.hpp"
#include "entt/entt.hpp"
#include <concepts>
namespace MEngine
{
class ISystem
{
  public:
    virtual ~ISystem() = default;

    virtual void Init() = 0;
    virtual void Tick(float deltaTime) = 0;
    virtual void Shutdown() = 0;
};
} // namespace MEngine