#pragma once
#include "Entity/Interface/IEntity.hpp"
#include <concepts>
namespace MEngine
{
template <typename TEntity, typename TKey = UUID>
    requires std::derived_from<TEntity, IEntity<TKey>>
class IWriteRepository
{
  public:
    virtual ~IWriteRepository() = default;
    virtual TEntity *Create() = 0;
    virtual bool Update(const TKey &id, const TEntity &delta) = 0;
    virtual bool Update(const TKey &id, const TEntity *delta) = 0;
    virtual bool Delete(const TKey &id) = 0;
};
} // namespace MEngine