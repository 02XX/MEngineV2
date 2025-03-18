#pragma once
#include "Buffer.hpp"
#include "Context.hpp"
#include "Logger.hpp"
#include "MEngine.hpp"
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
using SharedDescriptorSet = std::shared_ptr<vk::DescriptorSet>;
class MENGINE_API DescriptorManager final
{
  private:
    PoolSizesProportion mDefaultPoolSizesProportion;
    uint32_t mMaxDescriptorSize;
    std::vector<vk::UniqueDescriptorPool> mExhaustedPools;
    std::vector<vk::UniqueDescriptorPool> mAllocatablePools;

    vk::UniqueDescriptorPool &AcquireAllocatablePool();

  public:
    DescriptorManager(uint32_t maxDescriptorSize = 1000, PoolSizesProportion defaultPoolSizesProportion = {});

    std::vector<vk::UniqueDescriptorSet> AllocateUniqueDescriptorSet(
        std::vector<vk::DescriptorSetLayout> descriptorSetLayouts);
    std::vector<SharedDescriptorSet> AllocateSharedDescriptorSet(
        std::vector<vk::DescriptorSetLayout> descriptorSetLayouts);
    void ResetDescriptorPool();
    void SetDefaultPoolSizesProportion(PoolSizesProportion defaultPoolSizesProportion)
    {
        mDefaultPoolSizesProportion = defaultPoolSizesProportion;
    }
    void UpdateUniformDescriptorSet(const std::vector<Buffer> uniformBuffers, uint32_t binding,
                                    vk::DescriptorSet dstSet);
    void UpdateCombinedSamplerImageDescriptorSet(const std::vector<ImageDescriptor> &imageDescriptors, uint32_t binding,
                                                 vk::DescriptorSet dstSet);
};

} // namespace MEngine