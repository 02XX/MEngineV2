#pragma once
#include <filesystem>
#include <vulkan/vulkan.hpp>
namespace MEngine
{
enum class AssetType
{
    Folder,
    File,
};
struct AssetsComponent
{
    std::filesystem::path path;
    std::string name;
    AssetType type;
    vk::DescriptorSet iconDescriptorSet;
};
} // namespace MEngine