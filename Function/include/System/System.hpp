#pragma once
#include "ISystem.hpp"
namespace MEngine
{
class System : public ISystem
{
  protected:
    bool mIsInit = false;
    bool mIsShutdown = false;

  public:
    System() = default;
    virtual ~System() override = default;

    virtual void Init() override {};
    virtual void Tick(float deltaTime) override {};
    virtual void Shutdown() override {};

    virtual bool IsInit() const
    {
        return mIsInit;
    };
    virtual bool IsShutdown() const
    {
        return mIsShutdown;
    };
};
} // namespace MEngine