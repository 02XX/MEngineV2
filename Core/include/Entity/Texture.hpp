#pragma once

#include "Context.hpp"
#include "Entity.hpp"
#include "Image.hpp"
#include "ImageFactory.hpp"
#include "Interface/IEntity.hpp"
#include "Interface/ITexture.hpp"
#include "NoCopyable.hpp"
#include "SamplerManager.hpp"
#include "stb_image.h"
#include <cstdint>
#include <filesystem>
#include <memory>

namespace MEngine
{
class Texture final : public Entity<UUID>, public ITexture
{
    friend nlohmann::adl_serializer<MEngine::Texture>;
    friend class TextureRepository;

  private:
    std::filesystem::path imagePath{};
    uint32_t mWidth;    // 纹理宽度
    uint32_t mHeight;   // 纹理高度
    uint32_t mChannels; // 纹理通道数
    // Vulkan Resources
    UniqueImage mImage;             // Vulkan 纹理图像
    vk::UniqueImageView mImageView; // Vulkan 纹理图像视图
    vk::UniqueSampler mSampler;     // Vulkan 纹理采样器

  public:
    Texture();
    // Getters
    inline vk::Image GetImage() const override
    {
        return mImage->GetHandle();
    }
    inline vk::ImageView GetImageView() const override
    {
        return mImageView.get();
    }
    inline vk::Sampler GetSampler() const override
    {
        return mSampler.get();
    }
    inline const std::filesystem::path &GetImagePath() const override
    {
        return imagePath;
    }
    inline uint32_t GetWidth() const
    {
        return mWidth;
    }
    inline uint32_t GetHeight() const
    {
        return mHeight;
    }
    inline uint32_t GetChannels() const
    {
        return mChannels;
    }
    // Setters
    void SetImagePath(const std::filesystem::path &path) override
    {
        imagePath = path;
    };
};
} // namespace MEngine

namespace nlohmann
{
template <> struct adl_serializer<MEngine::Texture>
{
    static void to_json(json &j, const MEngine::Texture &texture)
    {
        auto &entity = static_cast<const MEngine::ITexture &>(texture);
        j = entity;
        j["id"] = texture.GetID();
        j["imagePath"] = texture.GetImagePath().string();
        j["width"] = texture.GetWidth();
        j["height"] = texture.GetHeight();
        j["channels"] = texture.GetChannels();
    }
    static void from_json(const json &j, MEngine::Texture &texture)
    {
        auto &entity = static_cast<MEngine::ITexture &>(texture);
        j.get_to(entity);
        std::string imagePath = j.at("imagePath").get<std::string>();
        texture.imagePath = imagePath;
        texture.mWidth = j.at("width").get<uint32_t>();
        texture.mHeight = j.at("height").get<uint32_t>();
        texture.mChannels = j.at("channels").get<uint32_t>();
    }
};
} // namespace nlohmann