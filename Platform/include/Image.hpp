#pragma once
#include "Context.hpp"
#include "MEngine.hpp"
#include "VMA.hpp"
#include <vulkan/vulkan.hpp>


namespace MEngine
{
class Image final
{
  public:
    Image(const vk::ImageCreateInfo &imageInfo, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags flags = 0);

    Image(const Image &) = delete;
    Image &operator=(const Image &) = delete;

    Image(Image &&other) noexcept;
    Image &operator=(Image &&other) noexcept;

    ~Image();

    const vk::Image &GetImage() const;
    const VmaAllocationInfo &GetAllocationInfo() const;

  private:
    void Release();

    VmaAllocation mAllocation;
    VmaAllocationInfo mAllocationInfo;
    vk::Image mImage;
};

using UniqueImage = std::unique_ptr<Image>;
using SharedImage = std::shared_ptr<Image>;
} // namespace MEngine