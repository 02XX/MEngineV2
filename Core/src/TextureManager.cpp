#include "TextureManager.hpp"

namespace MEngine
{
TextureManager::TextureManager(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                               std::shared_ptr<ImageFactory> imageFactory,
                               std::shared_ptr<SamplerManager> samplerManager)
    : mLogger(logger), mContext(context), mImageFactory(imageFactory), mSamplerManager(samplerManager)
{
}
std::shared_ptr<Texture> TextureManager::GetTexture(std::filesystem::path path)
{
    if (path.empty())
    {
        mLogger->Error("Texture path is empty");
        return nullptr;
    }
    if (path.extension() != ".png" && path.extension() != ".jpg" && path.extension() != ".jpeg")
    {
        mLogger->Error("Unsupported texture format: {}", path.extension().string());
        return nullptr;
    }
    if (std::filesystem::exists(path) == false)
    {
        mLogger->Error("Texture file not found: {}", path.string());
        return nullptr;
    }
    if (mTextures.find(path) != mTextures.end())
    {
        return mTextures[path];
    }
    auto texture = std::make_shared<Texture>(mContext, mImageFactory, mSamplerManager, path);
    auto id = std::hash<std::string>{}(path.string());
    mTextures[path] = texture;
    return mTextures[path];
}
std::shared_ptr<Texture> TextureManager::GetDefaultTexture()
{
    if (defaultTexture == nullptr)
    {
        defaultTexture =
            std::make_shared<Texture>(mContext, mImageFactory, mSamplerManager,
                                      std::filesystem::current_path() / "Resource" / "Material" / "DefaultAlbedo.png");
    }
    return defaultTexture;
}
} // namespace MEngine