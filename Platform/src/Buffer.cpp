#include "Buffer.hpp"

namespace MEngine
{
Buffer::Buffer(vk::DeviceSize size, vk::BufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage,
               VmaAllocationCreateFlags flags)
{
    auto &context = Context::Instance();
    vk::BufferCreateInfo bufferCreateInfo{};
    // 默认为共享模式
    bufferCreateInfo.setSize(size).setUsage(bufferUsage).setSharingMode(vk::SharingMode::eExclusive);

    VmaAllocationCreateInfo allocationCreateInfo{};
    allocationCreateInfo.flags = flags;
    allocationCreateInfo.usage = memoryUsage;
    allocationCreateInfo.priority = 1.0f;

    VkBuffer vkBuffer = mBuffer;
    auto result = vmaCreateBuffer(context.GetVmaAllocator(),                            // VMA 分配器
                                  &static_cast<VkBufferCreateInfo &>(bufferCreateInfo), // 转换为 C 结构体
                                  &allocationCreateInfo,                                // 内存分配策略
                                  &vkBuffer,                                            // 输出缓冲区句柄
                                  &mAllocation,                                         // 输出内存分配句柄
                                  &mAllocationInfo // 输出内存分配详细信息
    );
    if (result != VK_SUCCESS)
    {
        LogE("Failed to create buffer: {}", vk::to_string(vk::Result(result)));
        throw std::runtime_error("Failed to create buffer");
    }
    mBuffer = vk::Buffer(vkBuffer);
}
Buffer::Buffer(Buffer &&other) noexcept
    : mBuffer(std::exchange(other.mBuffer, nullptr)), mAllocation(std::exchange(other.mAllocation, nullptr)),
      mAllocationInfo(std::exchange(other.mAllocationInfo, {}))
{
}
Buffer &Buffer::operator=(Buffer &&other) noexcept
{
    if (this != &other)
    {
        Release();
        mBuffer = std::exchange(other.mBuffer, nullptr);
        mAllocation = std::exchange(other.mAllocation, nullptr);
        mAllocationInfo = std::exchange(other.mAllocationInfo, {});
    }
    return *this;
}

Buffer::~Buffer()
{
    Release();
}
void Buffer::Release()
{
    vmaDestroyBuffer(Context::Instance().GetVmaAllocator(), mBuffer, mAllocation);
}
const vk::Buffer &Buffer::GetBuffer() const
{
    return mBuffer;
}

const VmaAllocationInfo &Buffer::GetAllocationInfo() const
{
    return mAllocationInfo;
}

} // namespace MEngine