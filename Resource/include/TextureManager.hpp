#pragma once
#include "Context.hpp"
#include "Interface/ILogger.hpp"
#include "NoCopyable.hpp"
#include "Texture.hpp"
#include <cstdint>
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
    std::unordered_map<uint32_t, std::weak_ptr<Texture>> mTextures; // 纹理
  public:
    TextureManager(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                   std::shared_ptr<ImageFactory> imageFactory, std::shared_ptr<SamplerManager> samplerManager);
    std::shared_ptr<Texture> CreateTexture(std::filesystem::path path, TextureType type);
    std::shared_ptr<Texture> GetTexture(uint32_t id);
};
} // namespace MEngine