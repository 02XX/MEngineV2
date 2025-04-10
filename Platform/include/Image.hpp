#pragma once
#include "Context.hpp"
#include "MEngine.hpp"
#include "VMA.hpp"
#include <memory>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>

namespace MEngine
{
class Image final
{
  private:
    // DI
    std::shared_ptr<Context> mContext;

  private:
    VmaAllocation mAllocation;
    VmaAllocationInfo mAllocationInfo;
    vk::Image mImage;

    vk::Format mFormat;
    vk::Extent3D mExtent;
    vk::ImageUsageFlags mUsageFlags;
    vk::ImageTiling mTiling;
    vk::ImageType mImageType;
    vk::SampleCountFlagBits mSamples;
    uint32_t mMipLevels;
    uint32_t mArrayLayers;

  public:
    Image(std::shared_ptr<Context> context, const vk::ImageCreateInfo &imageInfo, VmaMemoryUsage memoryUsage,
          VmaAllocationCreateFlags flags = 0);
    Image(const Image &) = delete;
    Image &operator=(const Image &) = delete;
    Image(Image &&other) noexcept;
    Image &operator=(Image &&other) noexcept;
    ~Image();

  public:
    vk::Image GetHandle() const;
    VmaAllocationInfo GetAllocationInfo() const;

    vk::Format GetFormat() const;
    vk::Extent3D GetExtent() const;
    vk::ImageUsageFlags GetUsage() const;
    vk::ImageTiling GetTiling() const;
    vk::ImageType GetImageType() const;
    vk::SampleCountFlagBits GetSamples() const;
    uint32_t GetMipLevels() const;
    uint32_t GetArrayLayers() const;

  private:
    void Release();
};

using UniqueImage = std::unique_ptr<Image>;
} // namespace MEngine