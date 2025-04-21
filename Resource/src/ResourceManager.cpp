// #include "ResourceManager.hpp"

// namespace MEngine
// {
// void ResourceManager::LoadResource(const std::filesystem::path &path, std::shared_ptr<AssetNode> parent)
// {
//     for (const auto &entry : std::filesystem::directory_iterator(path))
//     {
//         // Check if the entry is a directory or a file
//         if (!entry.is_directory() && !entry.is_regular_file())
//             continue;
//         auto assetNode = std::make_shared<AssetNode>();
//         assetNode->path = entry.path();
//         assetNode->name = entry.path().filename().string();
//         assetNode->parent = parent;
//         if (entry.is_directory())
//         {
//             assetNode->type = AssetType::Folder;
//             assetNode->id = UUIDGenerator()();

//             LoadResource(entry.path(), assetNode);
//         }
//         else
//         {
//             assetNode->type = GetAssetTypeFromExtension(entry.path());
//             switch (assetNode->type)
//             {
//             case AssetType::PBRMaterial: {
//                 // PBR
//                 auto material = mPBRMaterialRepository->LoadFromFile(entry.path());
//                 assetNode->id = material->GetID();
//                 break;
//             }
//             case AssetType::Texture2D: {
//                 auto texture = mTexture2DRepository->LoadFromFile(entry.path());
//                 assetNode->id = texture->GetID();
//                 break;
//             }
//             default:
//                 break;
//             }
//         }
//         parent->children.push_back(assetNode);
//         mAssetMap[assetNode->path] = assetNode;
//     }
// }
// AssetType ResourceManager::GetAssetTypeFromExtension(const std::filesystem::path &path)
// {
//     auto ext = path.extension().string();
//     if (ext == ".pbrmat" || ext == ".phongmat")
//         return AssetType::PBRMaterial;
//     if (ext == ".tex2D" || ext == ".png" || ext == ".jpg")
//         return AssetType::Texture2D;
//     if (ext == ".glb" || ext == ".fbx")
//         return AssetType::Model;
//     if (ext == ".anim")
//         return AssetType::Animation;
//     if (ext == ".shader")
//         return AssetType::Shader;
//     if (ext == ".wav" || ext == ".mp3")
//         return AssetType::Audio;
//     return AssetType::File;
// }

// } // namespace MEngine