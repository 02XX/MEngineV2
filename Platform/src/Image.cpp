#include "Image.hpp"
#include <utility>

namespace MEngine
{
Image::Image(std::shared_ptr<Context> context, const vk::ImageCreateInfo &imageInfo, VmaMemoryUsage memoryUsage,
             VmaAllocationCreateFlags flags)
    : mContext(context)
{

    VmaAllocationCreateInfo allocationCreateInfo{};
    allocationCreateInfo.flags = flags;
    allocationCreateInfo.usage = memoryUsage;
    allocationCreateInfo.priority = 1.0f;
    VkImage image = mImage;
    auto result = vmaCreateImage(mContext->GetVmaAllocator(), &static_cast<const VkImageCreateInfo &>(imageInfo),
                                 &allocationCreateInfo, &image, &mAllocation, &mAllocationInfo);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create image");
    }
    mImage = vk::Image(image);

    mFormat = imageInfo.format;
    mExtent = imageInfo.extent;
    mUsageFlags = imageInfo.usage;
    mImageType = imageInfo.imageType;
    mTiling = imageInfo.tiling;
    mSamples = imageInfo.samples;
    mMipLevels = imageInfo.mipLevels;
    mArrayLayers = imageInfo.arrayLayers;
}

Image::Image(Image &&other) noexcept
    : mImage(std::exchange(other.mImage, nullptr)), mAllocation(std::exchange(other.mAllocation, nullptr)),
      mAllocationInfo(std::exchange(other.mAllocationInfo, {})), mFormat(std::exchange(other.mFormat, vk::Format{})),
      mExtent(std::exchange(other.mExtent, {})), mUsageFlags(std::exchange(other.mUsageFlags, {})),
      mImageType(std::exchange(other.mImageType, {})), mTiling(std::exchange(other.mTiling, {})),
      mSamples(std::exchange(other.mSamples, {})), mMipLevels(std::exchange(other.mMipLevels, {})),
      mArrayLayers(std::exchange(other.mArrayLayers, {})), mContext(std::exchange(other.mContext, nullptr))
{
}

Image &Image::operator=(Image &&other) noexcept
{
    if (this != &other)
    {
        Release();
        mContext = std::exchange(other.mContext, nullptr);
        mImage = std::exchange(other.mImage, nullptr);
        mAllocation = std::exchange(other.mAllocation, nullptr);
        mAllocationInfo = std::exchange(other.mAllocationInfo, {});
        mFormat = std::exchange(other.mFormat, vk::Format{});
        mExtent = std::exchange(other.mExtent, {});
        mUsageFlags = std::exchange(other.mUsageFlags, {});
        mImageType = std::exchange(other.mImageType, {});
        mTiling = std::exchange(other.mTiling, {});
        mSamples = std::exchange(other.mSamples, {});
        mMipLevels = std::exchange(other.mMipLevels, {});
        mArrayLayers = std::exchange(other.mArrayLayers, {});
    }
    return *this;
}
Image::~Image()
{
    Release();
}

vk::Image Image::GetHandle() const
{
    return mImage;
}

VmaAllocationInfo Image::GetAllocationInfo() const
{
    return mAllocationInfo;
}
vk::Format Image::GetFormat() const
{
    return mFormat;
}
vk::Extent3D Image::GetExtent() const
{
    return mExtent;
}
vk::ImageUsageFlags Image::GetUsage() const
{
    return mUsageFlags;
}
vk::ImageTiling Image::GetTiling() const
{
    return mTiling;
}
vk::ImageType Image::GetImageType() const
{
    return mImageType;
}
vk::SampleCountFlagBits Image::GetSamples() const
{
    return mSamples;
}
uint32_t Image::GetMipLevels() const
{
    return mMipLevels;
}
uint32_t Image::GetArrayLayers() const
{
    return mArrayLayers;
}
void Image::Release()
{
    vmaDestroyImage(mContext->GetVmaAllocator(), mImage, mAllocation);
}
} // namespace MEngine