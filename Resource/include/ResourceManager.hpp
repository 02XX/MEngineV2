#pragma once
#include "Buffer.hpp"
#include "Image.hpp"
#include "Logger.hpp"
#include "MEngine.hpp"
#include "PipelineLayoutManager.hpp"
#include "PipelineManager.hpp"
#include "SharedHandle.hpp"
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan_handles.hpp>

namespace MEngine
{
enum class ResourceType
{
    Texture,
    Pipeline,
    Mesh,
    Material,
    Shader
};

class MENGINE_API ResourceManager
{
  private:
    std::unordered_map<uint32_t, UniqueBuffer> mBuffers;
    std::unordered_map<uint32_t, UniqueImage> mImages;

  public:
    ResourceManager();
    ResourceManager(const ResourceManager &) = delete;
    ResourceManager &operator=(const ResourceManager &) = delete;
    ResourceManager(ResourceManager &&) = delete;
    ResourceManager &operator=(ResourceManager &&) = delete;
    ~ResourceManager() = default;

    vk::Buffer GetBuffer(uint32_t id);
    vk::Image GetImage(uint32_t id);
};
} // namespace MEngine