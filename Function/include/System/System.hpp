#pragma once
#include "Context.hpp"
#include "ISystem.hpp"
#include "Interface/IConfigure.hpp"
#include "Interface/ILogger.hpp"
#include <memory>

namespace MEngine
{
class System : public ISystem
{
  protected:
    // DI
    std::shared_ptr<ILogger> mLogger;
    std::shared_ptr<Context> mContext;
    std::shared_ptr<IConfigure> mConfigure;
    std::shared_ptr<entt::registry> mRegistry;

  protected:
    bool mIsInit = false;
    bool mIsShutdown = false;

  public:
    System(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context, std::shared_ptr<IConfigure> configure,
           std::shared_ptr<entt::registry> registry)
        : mLogger(logger), mContext(context), mConfigure(configure), mRegistry(registry)
    {
    }
    virtual ~System() = default;
    // auto GetEntitiesView()
    // {
    //     return mRegistry->view<Component...>();
    // };
    virtual void Init() override {};
    virtual void Tick(float deltaTime) override {};
    virtual void Shutdown() override {};
    inline virtual bool IsInit() const
    {
        return mIsInit;
    };
    inline virtual bool IsShutdown() const
    {
        return mIsShutdown;
    };
};
} // namespace MEngine