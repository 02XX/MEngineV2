#pragma once
#include "Context.hpp"
#include "MEngine.hpp"
#include "VMA.hpp"
#include <memory>
#include <vulkan/vulkan.hpp>
namespace MEngine
{
class Buffer final
{
  private:
    // DI
    std::shared_ptr<Context> mContext;

  private:
    VmaAllocation mAllocation;
    VmaAllocationInfo mAllocationInfo;
    vk::Buffer mBuffer;

    vk::DeviceSize mBufferSize;
    vk::BufferUsageFlags mBufferUsageFlags;

  public:
    Buffer(std::shared_ptr<Context> context, vk::DeviceSize size, vk::BufferUsageFlags bufferUsage,
           VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags flags = 0);
    Buffer(const Buffer &) = delete;
    Buffer &operator=(const Buffer &) = delete;
    Buffer(Buffer &&other) noexcept;
    Buffer &operator=(Buffer &&other) noexcept;
    ~Buffer();

  public:
    vk::Buffer GetHandle() const;
    VmaAllocationInfo GetAllocationInfo() const;
    vk::DeviceSize GetSize() const;
    vk::BufferUsageFlags GetUsage() const;

  private:
    void Release();
};
using UniqueBuffer = std::unique_ptr<Buffer>;
} // namespace MEngine