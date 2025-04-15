#pragma once
#include "Entity/Interface/IEntity.hpp"
#include <concepts>
#include <memory>
#include <vector>
namespace MEngine
{
template <typename Entity, typename Metadata, typename ID>
    requires std::derived_from<Entity, IEntity> && std::derived_from<Metadata, IMetadata<ID>>
class IReadOnlyRepository
{
  public:
    virtual ~IReadOnlyRepository() = default;
    virtual Entity *Get(const ID &id) = 0;
    virtual std::vector<Entity *> GetAll() = 0;
    virtual std::vector<Entity *> GetByName(const std::string &name) = 0;
};
} // namespace MEngine