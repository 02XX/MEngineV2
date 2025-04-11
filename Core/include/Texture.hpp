#pragma once

#include "Context.hpp"
#include "Image.hpp"
#include "ImageFactory.hpp"
#include "NoCopyable.hpp"
#include "SamplerManager.hpp"
#include "stb_image.h"
#include <cstdint>
#include <filesystem>
#include <memory>
namespace MEngine
{
enum class TextureType
{
    BaseColor,
    Albedo,
    Normal,
    Specular,
    Emissive,
    Height,
    Roughness,
    Metallic,
    AmbientOcclusion
};
class Texture final : public NoCopyable
{
  private:
    // DI
    std::shared_ptr<Context> mContext;
    std::shared_ptr<ImageFactory> mImageFactory;
    std::shared_ptr<SamplerManager> mSamplerManager;

  private:
    UniqueImage mImage;             // Vulkan 纹理图像
    vk::UniqueImageView mImageView; // Vulkan 纹理图像视图
    vk::UniqueSampler mSampler;     // Vulkan 纹理采样器
    std::filesystem::path mPath;    // 纹理路径
    uint32_t mWidth;                // 纹理宽度
    uint32_t mHeight;               // 纹理高度
    uint32_t mChannels;             // 纹理通道数
  public:
    Texture(std::shared_ptr<Context> context,
            std::shared_ptr<ImageFactory> imageFactory, std::shared_ptr<SamplerManager> samplerManager,
            const std::filesystem::path &path);
    vk::ImageView GetImageView() const;
    vk::Sampler GetSampler() const;
    vk::Image GetImage() const;
    void LoadTexture(const std::filesystem::path &path);
};
} // namespace MEngine