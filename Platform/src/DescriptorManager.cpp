#include "DescriptorManager.hpp"

namespace MEngine
{

DescriptorManager::DescriptorManager(uint32_t maxDescriptorSize, PoolSizesProportion defaultPoolSizesProportion)
    : mMaxDescriptorSize(maxDescriptorSize), mDefaultPoolSizesProportion(defaultPoolSizesProportion)
{
}
vk::UniqueDescriptorPool &DescriptorManager::AcquireAllocatablePool()
{
    auto &context = Context::Instance();
    if (!mAllocatablePools.empty())
    {
        return mAllocatablePools.back();
    }
    std::vector<vk::DescriptorPoolSize> descriptorPoolSize;
    descriptorPoolSize.reserve(mDefaultPoolSizesProportion.proportion.size());
    for (auto &proportion : mDefaultPoolSizesProportion.proportion)
    {
        descriptorPoolSize.emplace_back(proportion.first,
                                        static_cast<uint32_t>(proportion.second * mMaxDescriptorSize));
    }
    vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo;
    descriptorPoolCreateInfo
        .setPoolSizes(descriptorPoolSize) // 设置池中各类描述符的数量
        .setMaxSets(mMaxDescriptorSize);  // 设置池最多可分配的 Descriptor Set 数量
    mAllocatablePools.push_back(context.GetDevice()->createDescriptorPoolUnique(descriptorPoolCreateInfo));
    return mAllocatablePools.back();
}
std::vector<vk::UniqueDescriptorSet> DescriptorManager::AllocateDescriptorSet(
    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts)
{
    auto &allocatablePool = AcquireAllocatablePool();
    auto &context = Context::Instance();
    vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo;
    descriptorSetAllocateInfo.setDescriptorPool(allocatablePool.get())
        .setDescriptorSetCount(descriptorSetLayouts.size())
        .setSetLayouts(descriptorSetLayouts);
    try
    {
        auto result = context.GetDevice()->allocateDescriptorSetsUnique(descriptorSetAllocateInfo);
        return result;
    }
    catch (vk::OutOfPoolMemoryError &)
    {
        mExhaustedPools.push_back(std::move(allocatablePool));
        mAllocatablePools.erase(std::remove_if(mAllocatablePools.begin(), mAllocatablePools.end(),
                                               [&allocatablePool](const vk::UniqueDescriptorPool &pool) {
                                                   return pool.get() == allocatablePool.get();
                                               }),
                                mAllocatablePools.end());
        return AllocateDescriptorSet(descriptorSetLayouts);
    }
    catch (vk::FragmentedPoolError &)
    {
        mExhaustedPools.push_back(std::move(allocatablePool));
        mAllocatablePools.erase(std::remove_if(mAllocatablePools.begin(), mAllocatablePools.end(),
                                               [&allocatablePool](const vk::UniqueDescriptorPool &pool) {
                                                   return pool.get() == allocatablePool.get();
                                               }),
                                mAllocatablePools.end());
        return AllocateDescriptorSet(descriptorSetLayouts);
    }
}

void DescriptorManager::ResetDescriptorPool()
{
    auto &context = Context::Instance();
    for (auto &exhaustedPool : mExhaustedPools)
    {
        context.GetDevice()->resetDescriptorPool(exhaustedPool.get());
        mAllocatablePools.push_back(std::move(exhaustedPool));
    }
    mExhaustedPools.clear();
}

void DescriptorManager::UpdateUniformDescriptorSet(const std::vector<Buffer> uniformBuffers, uint32_t binding,
                                                   vk::DescriptorSet dstSet)
{
    auto &context = Context::Instance();
    std::vector<vk::DescriptorBufferInfo> descriptorBufferInfos;
    descriptorBufferInfos.reserve(uniformBuffers.size());
    for (auto &uniformBuffer : uniformBuffers)
    {
        vk::DescriptorBufferInfo descriptorBufferInfo;
        descriptorBufferInfo.setBuffer(uniformBuffer.GetBuffer())
            .setOffset(0)
            .setRange(uniformBuffer.GetAllocationInfo().size);
        descriptorBufferInfos.push_back(descriptorBufferInfo);
    }

    vk::WriteDescriptorSet writer;
    writer.setBufferInfo(descriptorBufferInfos)
        .setDescriptorType(vk::DescriptorType::eUniformBuffer)
        .setDescriptorCount(uniformBuffers.size())
        .setDstArrayElement(0)
        .setDstBinding(binding)
        .setDstSet(dstSet);
    context.GetDevice()->updateDescriptorSets({writer}, {});
    LogD("Uniform descriptor set updated");
}

void DescriptorManager::UpdateCombinedSamplerImageDescriptorSet(const std::vector<ImageDescriptor> &imageDescriptors,
                                                                uint32_t binding, vk::DescriptorSet dstSet)
{
    auto &context = Context::Instance();
    vk::WriteDescriptorSet writer;
    std::vector<vk::DescriptorImageInfo> descriptorImageInfos;
    descriptorImageInfos.reserve(imageDescriptors.size());
    for (auto &imageDescriptor : imageDescriptors)
    {
        vk::DescriptorImageInfo descriptorImageInfo;
        descriptorImageInfo.setSampler(imageDescriptor.sampler)
            .setImageView(imageDescriptor.imageView)
            .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
        descriptorImageInfos.push_back(descriptorImageInfo);
    }
    writer.setImageInfo(descriptorImageInfos)
        .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
        .setDescriptorCount(imageDescriptors.size())
        .setDstArrayElement(0)
        .setDstBinding(binding)
        .setDstSet(dstSet);
    context.GetDevice()->updateDescriptorSets({writer}, {});
    LogD("Combined sampler image descriptor set updated");
}

} // namespace MEngine