#pragma once
#include "Entity/Interface/IEntity.hpp"
#include <concepts>
#include <memory>
#include <vector>
namespace MEngine
{
template <typename Entity, typename Metadata, typename ID>
    requires std::derived_from<Entity, IEntity> && std::derived_from<Metadata, IMetadata<ID>>
class IWriteRepository
{
  public:
    virtual ~IWriteRepository() = default;
    virtual std::shared_ptr<Entity> Create(const Metadata &metadata) = 0;
    virtual bool Update(const ID &id, const Metadata &delta) = 0;
    virtual bool Delete(const ID &id) = 0;
};
} // namespace MEngine