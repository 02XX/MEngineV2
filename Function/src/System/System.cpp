#include "System/System.hpp"

namespace MEngine
{
System::System(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context, std::shared_ptr<IConfigure> configure,
               std::shared_ptr<entt::registry> registry)
    : mLogger(logger), mContext(context), mConfigure(configure), mRegistry(registry)
{
}
} // namespace MEngine