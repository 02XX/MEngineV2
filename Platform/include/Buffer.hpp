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
  public:
    Buffer(vk::DeviceSize size, vk::BufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage,
           VmaAllocationCreateFlags flags = 0);
    Buffer(const Buffer &) = delete;
    Buffer &operator=(const Buffer &) = delete;
    Buffer(Buffer &&other) noexcept;
    Buffer &operator=(Buffer &&other) noexcept;
    ~Buffer();
    const vk::Buffer &GetBuffer() const;
    const VmaAllocationInfo &GetAllocationInfo() const;

  private:
    VmaAllocation mAllocation;
    VmaAllocationInfo mAllocationInfo;
    vk::Buffer mBuffer;

    void Release();
};

using UniqueBuffer = std::unique_ptr<Buffer>;
using SharedBuffer = std::shared_ptr<Buffer>;
} // namespace MEngine