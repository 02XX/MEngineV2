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
    ~System() override = default;

    void Init() override {};
    void Tick(float deltaTime) override {};
    void Shutdown() override {};

    bool IsInit() const
    {
        return mIsInit;
    };
    bool IsShutdown() const
    {
        return mIsShutdown;
    };
};
} // namespace MEngine