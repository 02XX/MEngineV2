#pragma once
#include "Entity/Interface/IEntity.hpp"
#include "nlohmann/adl_serializer.hpp"

namespace MEngine
{
class IModel : public virtual IEntity<UUID>
{
    friend class nlohmann::adl_serializer<IModel>;

  public:
    virtual ~IModel() = default;
    virtual const std::filesystem::path &GetModelPath() const = 0;
};

} // namespace MEngine