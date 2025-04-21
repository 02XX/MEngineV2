// #pragma once
// #include "Buffer.hpp"

// #include "Entity/Interface/IEntity.hpp"
// #include "Entity/Texture2D.hpp"
// #include "Image.hpp"
// #include "MEngine.hpp"
// #include "NoCopyable.hpp"
// #include "PipelineLayoutManager.hpp"
// #include "PipelineManager.hpp"
// #include "Repository/Interface/IRepository.hpp"
// #include "Repository/PBRMaterialRepository.hpp"
// #include "Repository/Texture2DRepository.hpp"
// #include <cstdint>
// #include <entt/entt.hpp>
// #include <filesystem>
// #include <memory>
// #include <unordered_map>
// #include <vector>
// namespace MEngine
// {
// enum class AssetType
// {
//     Folder,
//     File,
//     PBRMaterial,
//     PhongMaterial,
//     Texture2D,
//     TextureCube,
//     Model,
//     Animation,
//     Shader,
//     Audio
// };
// struct AssetNode
// {
//     UUID id;
//     std::filesystem::path path;
//     std::string name;
//     AssetType type;
//     std::vector<std::shared_ptr<AssetNode>> children;
//     std::weak_ptr<AssetNode> parent;
// };
// class ResourceManager : public NoCopyable
// {
//   private:
//     std::shared_ptr<IRepository<PBRMaterial>> mPBRMaterialRepository;
//     std::shared_ptr<IRepository<Texture2D>> mTexture2DRepository;

//   private:
//     std::shared_ptr<AssetNode> mRootNode;
//     std::unordered_map<std::filesystem::path, std::shared_ptr<AssetNode>> mAssetMap;
//     std::unique_ptr<entt::registry> mAssetRegistry;

//   public:
//     ResourceManager(std::shared_ptr<IRepository<PBRMaterial>> pbrMaterialRepository,
//                     std::shared_ptr<IRepository<Texture2D>> texture2DRepository)
//         : mPBRMaterialRepository(pbrMaterialRepository), mTexture2DRepository(texture2DRepository)
//     {
//     }
//     void LoadResource(const std::filesystem::path &path, std::shared_ptr<AssetNode> parent);
//     AssetType GetAssetTypeFromExtension(const std::filesystem::path &path);
//     auto GetAllAssets() const
//     {
//         return mAssetMap;
//     }
//     auto GetRootNode() const
//     {
//         return mRootNode;
//     }
// };
// } // namespace MEngine