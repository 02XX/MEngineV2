#include "Image.hpp"

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
}

Image::Image(Image &&other) noexcept
    : mImage(std::exchange(other.mImage, nullptr)), mAllocation(std::exchange(other.mAllocation, nullptr)),
      mAllocationInfo(std::exchange(other.mAllocationInfo, {}))
{
}

Image &Image::operator=(Image &&other) noexcept
{
    if (this != &other)
    {
        Release();
        mImage = std::exchange(other.mImage, nullptr);
        mAllocation = std::exchange(other.mAllocation, nullptr);
        mAllocationInfo = std::exchange(other.mAllocationInfo, {});
    }
    return *this;
}
Image::~Image()
{
    Release();
}

vk::Image Image::GetImage() const
{
    return mImage;
}

VmaAllocationInfo Image::GetAllocationInfo() const
{
    return mAllocationInfo;
}

void Image::Release()
{
    vmaDestroyImage(mContext->GetVmaAllocator(), mImage, mAllocation);
}
} // namespace MEngine