#include "TextureManager.hpp"

namespace MEngine
{
TextureManager::TextureManager(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                               std::shared_ptr<ImageFactory> imageFactory,
                               std::shared_ptr<SamplerManager> samplerManager)
    : mLogger(logger), mContext(context), mImageFactory(imageFactory), mSamplerManager(samplerManager)
{
}
std::shared_ptr<Texture> TextureManager::CreateTexture(std::filesystem::path path, TextureType type)
{

    auto texture = std::make_shared<Texture>(mContext, mImageFactory, mSamplerManager, type, path);
    auto id = std::hash<std::string>{}(path.string());
    mTextures[id] = texture;
    return texture;
}

std::shared_ptr<Texture> TextureManager::GetTexture(uint32_t id)
{
    auto it = mTextures.find(id);
    if (it != mTextures.end())
    {
        auto texture = it->second.lock();
        if (texture)
        {
            return texture;
        }
        else
        {
            mTextures.erase(it);
        }
    }
    return nullptr;
}
} // namespace MEngine