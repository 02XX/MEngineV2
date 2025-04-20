#pragma once

#include "Entity/Interface/IEntity.hpp"
#include "Repository/Interface/IReadOnlyRepository.hpp"
#include "Repository/Interface/IStorage.hpp"
#include "Repository/Interface/IWriteRepository.hpp"

namespace MEngine
{
template <typename TEntity, typename TKey = UUID>
    requires std::derived_from<TEntity, IEntity<TKey>>
class IRepository : public IReadOnlyRepository<TEntity, TKey>,
                    public IWriteRepository<TEntity, TKey>,
                    public IStorage<TEntity>
{
  public:
    virtual ~IRepository() = default;
};
} // namespace MEngine