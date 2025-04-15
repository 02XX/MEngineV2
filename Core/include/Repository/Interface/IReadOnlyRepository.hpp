#pragma once
#include "Entity/Interface/IEntity.hpp"
#include <concepts>
#include <memory>
#include <vector>
namespace MEngine
{
template <typename TEntity, typename TKey = UUID>
    requires std::derived_from<TEntity, IEntity<TKey>>
class IReadOnlyRepository
{
  public:
    virtual ~IReadOnlyRepository() = default;
    virtual TEntity *Get(const TKey &id) = 0;
    virtual std::vector<TEntity *> GetAll() = 0;
    virtual std::vector<TEntity *> GetByName(const std::string &name) = 0;
};
} // namespace MEngine