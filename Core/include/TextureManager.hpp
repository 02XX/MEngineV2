#pragma once
#include "Context.hpp"
#include "Entity/Texture.hpp"
#include "Interface/ILogger.hpp"
#include "NoCopyable.hpp"
#include <cstdint>
#include <memory>
#include <unordered_map>

namespace MEngine
{
class TextureManager final : public NoCopyable
{
  private:
    // DI
    std::shared_ptr<ILogger> mLogger;
    std::shared_ptr<Context> mContext;
    std::shared_ptr<ImageFactory> mImageFactory;
    std::shared_ptr<SamplerManager> mSamplerManager;

  private:
    std::shared_ptr<Texture> defaultTexture; // 默认纹理

  private:
    std::unordered_map<std::filesystem::path, std::shared_ptr<Texture>> mTextures; // 纹理
  public:
    TextureManager(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                   std::shared_ptr<ImageFactory> imageFactory, std::shared_ptr<SamplerManager> samplerManager);
    std::shared_ptr<Texture> GetTexture(std::filesystem::path path);
    std::shared_ptr<Texture> GetDefaultTexture();
};
} // namespace MEngine