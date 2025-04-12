#include "Texture.hpp"

namespace MEngine
{
Texture::Texture(std::shared_ptr<Context> context, std::shared_ptr<ImageFactory> imageFactory,
                 std::shared_ptr<SamplerManager> samplerManager, TextureType type, const std::filesystem::path &path)
    : mContext(context), mPath(path), mImageFactory(imageFactory), mSamplerManager(samplerManager), mTextureType(type)
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
    return mImage->GetHandle();
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
        throw std::runtime_error("Failed to load texture image: " + path.string());
        return;
    }
    mWidth = static_cast<uint32_t>(width);
    mHeight = static_cast<uint32_t>(height);
    mChannels = 4;
    auto size = static_cast<vk::DeviceSize>(mWidth * mHeight * mChannels);
    // 3. 创建图像
    mImage = mImageFactory->CreateImage(ImageType::Texture2D, vk::Extent3D{mWidth, mHeight, 1}, size, imageData);
    // 4. 创建图像视图
    mImageView = mImageFactory->CreateImageView(mImage.get());
    // 5. 创建采样器
    mSampler = mSamplerManager->CreateUniqueSampler(vk::Filter::eLinear, vk::Filter::eLinear);
    stbi_image_free(imageData);
}

} // namespace MEngine