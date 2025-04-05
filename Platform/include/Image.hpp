#pragma once
#include "Context.hpp"
#include "MEngine.hpp"
#include "VMA.hpp"
#include <memory>
#include <vulkan/vulkan.hpp>

namespace MEngine
{
class Image final
{
  private:
    // DI
    std::shared_ptr<Context> mContext;

  public:
    Image(std::shared_ptr<Context> context, const vk::ImageCreateInfo &imageInfo, VmaMemoryUsage memoryUsage,
          VmaAllocationCreateFlags flags = 0);

    Image(const Image &) = delete;
    Image &operator=(const Image &) = delete;

    Image(Image &&other) noexcept;
    Image &operator=(Image &&other) noexcept;

    ~Image();

  public:
    vk::Image GetImage() const;
    VmaAllocationInfo GetAllocationInfo() const;

  private:
    void Release();

    VmaAllocation mAllocation;
    VmaAllocationInfo mAllocationInfo;
    vk::Image mImage;
};

using UniqueImage = std::unique_ptr<Image>;
} // namespace MEngine