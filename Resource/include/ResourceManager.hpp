#pragma once
#include "Buffer.hpp"

#include "Image.hpp"
#include "MEngine.hpp"
#include "NoCopyable.hpp"
#include "PipelineLayoutManager.hpp"
#include "PipelineManager.hpp"
#include "Texture.hpp"
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

namespace MEngine
{
class ResourceManager : public NoCopyable
{
  private:
    std::unordered_map<uint32_t, std::weak_ptr<Texture>> mTextures; // 纹理缓存

  public:
    ResourceManager();
    void LoadTexture(const std::filesystem::path &path);
};
} // namespace MEngine