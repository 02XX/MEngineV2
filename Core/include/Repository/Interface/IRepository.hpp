#pragma once

#include "Entity/Interface/IEntity.hpp"
#include "Repository/Interface/IReadOnlyRepository.hpp"
#include "Repository/Interface/IStorage.hpp"
#include "Repository/Interface/IWriteRepository.hpp"
#include "boost/uuid.hpp"

namespace MEngine
{
using UUID = boost::uuids::uuid;
template <typename Entity, typename Metadata, typename ID = UUID>
    requires std::derived_from<Entity, IEntity> && std::derived_from<Metadata, IMetadata<ID>>
class IRepository : public IReadOnlyRepository<Entity, Metadata, ID>,
                    public IWriteRepository<Entity, Metadata, ID>,
                    public IStorage<Entity>
{
  public:
    virtual ~IRepository() = default;
};
} // namespace MEngine