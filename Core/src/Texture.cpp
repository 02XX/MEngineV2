#include "Texture.hpp"

namespace MEngine
{
Texture::Texture(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                 std::shared_ptr<ImageManager> imageManager, std::shared_ptr<SamplerManager> samplerManager,
                 const std::filesystem::path &path)
    : mContext(context), mPath(path), mLogger(logger), mImageManager(imageManager), mSamplerManager(samplerManager)
{
    LoadTexture(path);
}

vk::ImageView Texture::GetImageView() const
{
    return mImageView.get();
}

vk::Sampler Texture::GetSampler() const
{
    return mSampler.get();
}

vk::Image Texture::GetImage() const
{
    return mImage->GetImage();
}

void Texture::LoadTexture(const std::filesystem::path &path)
{
    // 1. 加载图像
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true); // 翻转图像
    // 2. 加载图像数据
    auto imageData = stbi_load(path.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if (!imageData)
    {
        mLogger->Error("Failed to load texture: {}", path.string());
        return;
    }
    mWidth = static_cast<uint32_t>(width);
    mHeight = static_cast<uint32_t>(height);
    mChannels = 4;
    // 3. 创建图像
    mImage =
        mImageManager->CreateUniqueTexture2D(vk::Extent2D{mWidth, mHeight}, vk::Format::eR8G8B8A8Srgb, 1, imageData);
    // 4. 创建图像视图
    mImageView = mImageManager->CreateImageView(mImage->GetImage(), vk::Format::eR8G8B8A8Srgb);
    // 5. 创建采样器
    mSampler = mSamplerManager->CreateUniqueSampler(vk::Filter::eLinear, vk::Filter::eLinear);
    stbi_image_free(imageData);
}

} // namespace MEngine