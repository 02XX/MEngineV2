#include "Buffer.hpp"

namespace MEngine
{
Buffer::Buffer(std::shared_ptr<Context> context, vk::DeviceSize size, vk::BufferUsageFlags bufferUsage,
               VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags flags)
    : mContext(context), mBuffer(nullptr), mAllocation(nullptr)
{
    vk::BufferCreateInfo bufferCreateInfo{};
    // 默认为共享模式
    bufferCreateInfo.setSize(size).setUsage(bufferUsage).setSharingMode(vk::SharingMode::eExclusive);

    VmaAllocationCreateInfo allocationCreateInfo{};
    allocationCreateInfo.flags = flags;
    allocationCreateInfo.usage = memoryUsage;
    allocationCreateInfo.priority = 1.0f;

    VkBuffer vkBuffer = mBuffer;
    auto result = vmaCreateBuffer(mContext->GetVmaAllocator(),                          // VMA 分配器
                                  &static_cast<VkBufferCreateInfo &>(bufferCreateInfo), // 转换为 C 结构体
                                  &allocationCreateInfo,                                // 内存分配策略
                                  &vkBuffer,                                            // 输出缓冲区句柄
                                  &mAllocation,                                         // 输出内存分配句柄
                                  &mAllocationInfo                                      // 输出内存分配详细信息
    );
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create buffer");
    }
    mBuffer = vk::Buffer(vkBuffer);

    mBufferSize = size;
    mBufferUsageFlags = bufferUsage;
}
Buffer::Buffer(Buffer &&other) noexcept
    : mBuffer(std::exchange(other.mBuffer, nullptr)), mAllocation(std::exchange(other.mAllocation, nullptr)),
      mAllocationInfo(std::exchange(other.mAllocationInfo, {})), mBufferSize(std::exchange(other.mBufferSize, 0)),
      mBufferUsageFlags(std::exchange(other.mBufferUsageFlags, {})), mContext(std::exchange(other.mContext, nullptr))
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
        mBufferSize = std::exchange(other.mBufferSize, 0);
        mBufferUsageFlags = std::exchange(other.mBufferUsageFlags, {});
        mContext = std::exchange(other.mContext, nullptr);
    }
    return *this;
}

Buffer::~Buffer()
{
    Release();
}
void Buffer::Release()
{
    vmaDestroyBuffer(mContext->GetVmaAllocator(), mBuffer, mAllocation);
}
vk::Buffer Buffer::GetHandle() const
{
    return mBuffer;
}
VmaAllocationInfo Buffer::GetAllocationInfo() const
{
    return mAllocationInfo;
}
vk::DeviceSize Buffer::GetSize() const
{
    return mBufferSize;
}
vk::BufferUsageFlags Buffer::GetUsage() const
{
    return mBufferUsageFlags;
}

} // namespace MEngine