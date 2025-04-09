#pragma once
#include "Buffer.hpp"
#include "Context.hpp"
#include "Interface/ILogger.hpp"
#include "MEngine.hpp"
#include "NoCopyable.hpp"

#include <memory>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace MEngine
{
struct ImageDescriptor
{
    vk::ImageView imageView; // 图像视图
    vk::Sampler sampler;     // 采样器
};
struct PoolSizesProportion
{
    std::vector<std::pair<vk::DescriptorType, float>> proportion = {
        {vk::DescriptorType::eSampler, 0.5f},
        {vk::DescriptorType::eCombinedImageSampler, 4.0f},
        {vk::DescriptorType::eSampledImage, 4.0f},
        {vk::DescriptorType::eStorageImage, 1.0f},
        {vk::DescriptorType::eUniformBuffer, 2.0f},
        {vk::DescriptorType::eStorageBuffer, 2.0f},
        {vk::DescriptorType::eUniformBufferDynamic, 1.0f},
        {vk::DescriptorType::eStorageBufferDynamic, 1.0f},
    };
};
using UniqueDescriptorSet = vk::UniqueDescriptorSet;
class DescriptorManager final : public NoCopyable
{
  private:
    // DI
    std::shared_ptr<Context> mContext;
    std::shared_ptr<ILogger> mLogger;

  private:
    PoolSizesProportion mDefaultPoolSizesProportion;
    uint32_t mMaxDescriptorSize;
    std::vector<vk::UniqueDescriptorPool> mExhaustedPools;
    std::vector<vk::UniqueDescriptorPool> mAllocatablePools;

    vk::UniqueDescriptorPool &AcquireAllocatablePool();

  public:
    DescriptorManager(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                      uint32_t maxDescriptorSize = 1000, PoolSizesProportion defaultPoolSizesProportion = {});

    std::vector<UniqueDescriptorSet> AllocateUniqueDescriptorSet(
        std::vector<vk::DescriptorSetLayout> descriptorSetLayouts);
    void ResetDescriptorPool();
    void SetDefaultPoolSizesProportion(PoolSizesProportion defaultPoolSizesProportion)
    {
        mDefaultPoolSizesProportion = defaultPoolSizesProportion;
    }
    void UpdateUniformDescriptorSet(const std::vector<Buffer *> &uniformBuffers, uint32_t binding,
                                    vk::DescriptorSet dstSet);
    void UpdateCombinedSamplerImageDescriptorSet(std::vector<ImageDescriptor> imageDescriptors, uint32_t binding,
                                                 vk::DescriptorSet dstSet);
};

} // namespace MEngine