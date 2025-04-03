#include "Image.hpp"

namespace MEngine
{
Image::Image(const vk::ImageCreateInfo &imageInfo, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags flags)
{
    auto &context = Context::Instance();

    VmaAllocationCreateInfo allocationCreateInfo{};
    allocationCreateInfo.flags = flags;
    allocationCreateInfo.usage = memoryUsage;
    allocationCreateInfo.priority = 1.0f;
    VkImage image = mImage;
    auto result = vmaCreateImage(context.GetVmaAllocator(), &static_cast<const VkImageCreateInfo &>(imageInfo),
                                 &allocationCreateInfo, &image, &mAllocation, &mAllocationInfo);
    if (result != VK_SUCCESS)
    {
        LogE("Failed to create image: {}", vk::to_string(vk::Result(result)));
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

const vk::Image &Image::GetImage() const
{
    return mImage;
}

const VmaAllocationInfo &Image::GetAllocationInfo() const
{
    return mAllocationInfo;
}

void Image::Release()
{
    auto &context = Context::Instance();
    vmaDestroyImage(context.GetVmaAllocator(), mImage, mAllocation);
}
} // namespace MEngine