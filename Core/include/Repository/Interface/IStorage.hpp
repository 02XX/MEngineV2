#pragma once
#include "Entity/Interface/IEntity.hpp"
#include "nlohmann/json.hpp"
#include <concepts>
#include <memory>
#include <objidl.h>
#include <vector>

using Json = nlohmann::json;
namespace MEngine
{
template <typename TEntity, typename TKey = UUID>
    requires std::derived_from<TEntity, IEntity<TKey>>
class IStorage
{
  public:
    virtual ~IStorage() = default;
    virtual void SaveToFile(const std::filesystem::path &filePath, const TEntity &entity) = 0;
    virtual TEntity *LoadFromFile(const std::filesystem::path &filePath) = 0;
};
} // namespace MEngine