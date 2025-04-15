#pragma once

#include "Configure.hpp"
#include "Context.hpp"
#include "Interface/ILogger.hpp"
#include "Interface/IRepository.hpp"
#include <memory>
#include <unordered_map>

namespace MEngine
{
template <typename TEntity, typename TKey = UUID>
    requires std::derived_from<TEntity, IEntity<TKey>>
class Repository : public IRepository<TEntity, TKey>
{
  protected:
    // DI
    std::shared_ptr<ILogger> mLogger;
    std::shared_ptr<Context> mContext;
    std::shared_ptr<IConfigure> mConfigure;

  protected:
    std::unordered_map<TKey, std::unique_ptr<TEntity>> mEntities{};

  public:
    Repository(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context, std::shared_ptr<IConfigure> configure)
        : mLogger(logger), mContext(context), mConfigure(configure)
    {
    }
    virtual ~Repository() = default;
    virtual TEntity *Create() override
    {
        auto entity = std::make_unique<TEntity>();
        auto id = entity->GetID();
        Update(id, *Get(id));
        mEntities[id] = std::move(entity);
        return mEntities[id].get();
    }
    virtual TEntity *Get(const TKey &id) override
    {
        auto it = mEntities.find(id);
        if (it != mEntities.end())
        {
            return it->second.get();
        }
        return nullptr;
    }
    virtual std::vector<TEntity *> GetAll() override
    {
        std::vector<TEntity *> entities;
        for (auto &pair : mEntities)
        {
            entities.push_back(pair.second.get());
        }
        return entities;
    }
    virtual std::vector<TEntity *> GetByName(const std::string &name) override
    {
        std::vector<TEntity *> entities;
        for (auto &pair : mEntities)
        {
            if (pair.second->GetName() == name)
            {
                entities.push_back(pair.second.get());
            }
        }
        return entities;
    }
    virtual bool Update(const TKey &id, const TEntity &delta) override = 0;
    virtual bool Update(const TKey &id, const TEntity *delta) override
    {
        return Update(id, *delta);
    }
    virtual bool Delete(const TKey &id) override
    {
        auto it = mEntities.find(id);
        if (it != mEntities.end())
        {
            mEntities.erase(it);
            return true;
        }
        return false;
    }
    virtual void SaveToFile(const std::filesystem::path &filePath, const TEntity &entity) override
    {
        if (!CheckValidate(filePath))
        {
            mLogger->Error("Failed to save texture to file: {}", filePath.string());
            return;
        }
        std::ofstream file(filePath);
        if (!file.is_open())
        {
            mLogger->Error("Failed to open file: {}", filePath.string());
            return;
        }
        Json j = entity;
        file << j.dump(4); // Pretty print with 4 spaces
        file.close();
    }
    virtual TEntity *LoadFromFile(const std::filesystem::path &filePath) override
    {
        if (!CheckValidate(filePath))
        {
            mLogger->Error("Failed to load texture from file: {}", filePath.string());
            return nullptr;
        }
        std::ifstream file(filePath);
        if (!file.is_open())
        {
            mLogger->Error("Failed to open file: {}", filePath.string());
            return nullptr;
        }
        Json j;
        file >> j;
        auto entity = std::make_unique<TEntity>();
        j.get_to(*entity);
        if (Get(entity->GetID()) == nullptr)
        {
            auto newEntity = Create();
            newEntity = entity.get();
            Update(newEntity->GetID(), *newEntity);
        }
        return mEntities[entity->GetID()].get();
    }
    virtual bool CheckValidate(const std::filesystem::path &filePath) const = 0;
    virtual bool CheckValidate(const TEntity &delta) const = 0;
};
} // namespace MEngine