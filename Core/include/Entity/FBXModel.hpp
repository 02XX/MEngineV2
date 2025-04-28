#pragma once
#include "Buffer.hpp"
#include "Entity/Entity.hpp"
#include "Entity/Interface/IModel.hpp"
#include "Mesh.hpp"
#include <memory>

namespace MEngine
{
class FBXModel final : public IModel, public Entity<>
{
    friend nlohmann::adl_serializer<MEngine::FBXModel>;

  private:
    std::filesystem::path mModelPath{};
    std::vector<std::unique_ptr<Mesh>> mMeshes{};
    
  public:
    FBXModel() = default;
    // Getters
    inline const std::filesystem::path &GetModelPath() const override
    {
        return mModelPath;
    }

};
} // namespace MEngine