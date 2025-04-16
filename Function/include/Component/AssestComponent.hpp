#pragma once
#include "Component/Interface/IComponent.hpp"
#include "entt/entity/storage.hpp"
#include <filesystem>
#include <vulkan/vulkan.hpp>

namespace MEngine
{
enum class AssetType
{
    Folder,
    File,
    Material,
    Texture,
    Model,
    Animation,
};
struct AssetsComponent : public IComponent<>
{
    std::filesystem::path path;
    std::string name;
    AssetType type;
    vk::DescriptorSet iconDescriptorSet;
    AssetsComponent() = default;
    AssetsComponent(std::filesystem::path p, std::string n, AssetType t, vk::DescriptorSet icon)
        : path(std::move(p)), name(std::move(n)), type(t), iconDescriptorSet(icon)
    {
    }
};
} // namespace MEngine

// template <> struct entt::storage_type<MEngine::AssetsComponent>
// {
//     using type = entt::storage<MEngine::AssetsComponent>;
// };