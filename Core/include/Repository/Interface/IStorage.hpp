#pragma once
#include "Entity/Interface/IEntity.hpp"
#include <concepts>
#include <memory>
#include <objidl.h>
#include <vector>
namespace MEngine
{
template <typename TEntity, typename TKey = UUID>
    requires std::derived_from<TEntity, IEntity<TKey>>
class IStorage
{
  public:
    virtual ~IStorage() = default;
    virtual void SaveToFile(const std::filesystem::path &filePath) = 0;
    virtual std::shared_ptr<TEntity> LoadFromFile(const std::filesystem::path &filePath) = 0;
};
} // namespace MEngine