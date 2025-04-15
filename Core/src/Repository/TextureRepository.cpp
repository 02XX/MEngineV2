#include "Repository/TextureRepository.hpp"

namespace MEngine
{
TextureRepository::TextureRepository(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                                     std::shared_ptr<IConfigure> configure, std::shared_ptr<ImageFactory> imageFactory,
                                     std::shared_ptr<SamplerManager> samplerManager)
    : Repository<Texture>(logger, context, configure), mImageFactory(imageFactory), mSamplerManager(samplerManager)
{
    defaultTexture = std::shared_ptr<Texture>(Create());
}
Texture *TextureRepository::Create()
{
    auto texture = std::make_unique<Texture>();
    auto defaultImagePath = mConfigure->GetJson()["Texture"]["Default"].get<std::string>();
    if (!CheckValidate(defaultImagePath))
    {
        return nullptr;
    }
    texture->imagePath = defaultImagePath;
    mEntities[texture->GetID()] = std::move(texture);
    Update(texture->GetID(), *Get(texture->GetID()));
    return mEntities[texture->GetID()].get();
}
bool TextureRepository::Update(const UUID &id, const Texture &delta)
{
    if (!CheckValidate(delta))
    {
        return false;
    }
    auto it = mEntities.find(id);
    auto texture = Get(id);
    if (it != mEntities.end())
    {
        it->second->imagePath = delta.imagePath;
        int width, height, channels;
        stbi_set_flip_vertically_on_load(true);
        auto imageData = stbi_load(delta.imagePath.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
        if (!imageData)
        {
            mLogger->Error("Failed to load default texture image: {}", delta.imagePath.string().c_str());
            return false;
        }
        texture->mWidth = static_cast<uint32_t>(width);
        texture->mHeight = static_cast<uint32_t>(height);
        texture->mChannels = 4;
        auto size = static_cast<vk::DeviceSize>(texture->mWidth * texture->mHeight * texture->mChannels);
        texture->mImage = mImageFactory->CreateImage(
            ImageType::Texture2D, vk::Extent3D{texture->mWidth, texture->mHeight, 1}, size, imageData);
        texture->mImageView = mImageFactory->CreateImageView(texture->mImage.get());
        texture->mSampler = mSamplerManager->CreateUniqueSampler(vk::Filter::eLinear, vk::Filter::eLinear);
        stbi_image_free(imageData);
        return true;
    }
    mLogger->Info("Texture with ID {} not exist", id);
    return false;
}
bool TextureRepository::CheckValidate(const std::filesystem::path &filePath) const
{
    if (filePath.empty())
    {
        mLogger->Error("Default texture path is empty!");
        return false;
    }
    if (std::filesystem::exists(filePath) == false)
    {
        mLogger->Error("Default texture path is not exist!");
        return false;
    }
    if (std::filesystem::is_directory(filePath))
    {
        mLogger->Error("Default texture path is a directory!");
        return false;
    }
    if (filePath.extension() != ".png")
    {
        mLogger->Error("Default texture path is not a png file!");
        return false;
    }
    return true;
}
bool TextureRepository::CheckValidate(const Texture &delta) const
{
    // Check imagePath
    return CheckValidate(delta.imagePath);
    // TODO: CheckValidate other members
}
} // namespace MEngine