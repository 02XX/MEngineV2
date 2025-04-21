#pragma once
#include "Component/Interface/IComponent.hpp"
#include "entt/entity/fwd.hpp"
#include "entt/entity/storage.hpp"
#include <filesystem>
#include <memory>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>
namespace MEngine
{
enum class AssetType
{
    Folder,
    File,
    PBRMaterial,
    PhongMaterial,
    Texture2D,
    TextureCube,
    Model,
    Animation,
    Shader,
    Audio
};
struct AssetsComponent : public IComponent<>
{
    std::filesystem::path path;
    std::string name;
    AssetType type;
    std::vector<entt::entity> children;
    entt::entity parent;
    vk::DescriptorSet iconDescriptorSet;
};
} // namespace MEngine

// template <> struct entt::storage_type<MEngine::AssetsComponent>
// {
//     using type = entt::storage<MEngine::AssetsComponent>;
// };