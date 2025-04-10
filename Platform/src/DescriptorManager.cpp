#include "DescriptorManager.hpp"

namespace MEngine
{

DescriptorManager::DescriptorManager(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                                     uint32_t maxDescriptorSize, PoolSizesProportion defaultPoolSizesProportion)
    : mMaxDescriptorSize(maxDescriptorSize), mDefaultPoolSizesProportion(defaultPoolSizesProportion), mContext(context),
      mLogger(logger)
{
}
vk::UniqueDescriptorPool &DescriptorManager::AcquireAllocatablePool()
{
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
        .setPoolSizes(descriptorPoolSize)                                // 设置池中各类描述符的数量
        .setMaxSets(mMaxDescriptorSize)                                  // 设置池最多可分配的 Descriptor Set 数量
        .setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet); // 设置池的标志位
    auto descriptorPool = mContext->GetDevice().createDescriptorPoolUnique(descriptorPoolCreateInfo);
    mAllocatablePools.push_back(std::move(descriptorPool));
    return mAllocatablePools.back();
}
std::vector<vk::DescriptorSet> DescriptorManager::AllocateUniqueDescriptorSet(
    const std::vector<vk::DescriptorSetLayout> &descriptorSetLayouts)
{
    auto &allocatablePool = AcquireAllocatablePool();
    vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo;
    descriptorSetAllocateInfo.setDescriptorPool(allocatablePool.get())
        .setDescriptorSetCount(descriptorSetLayouts.size())
        .setSetLayouts(descriptorSetLayouts);
    try
    {
        auto descriptorSet = mContext->GetDevice().allocateDescriptorSetsUnique(descriptorSetAllocateInfo);
        std::vector<vk::DescriptorSet> results;
        mDescriptorSets.reserve(descriptorSet.size());
        for (auto &set : descriptorSet)
        {
            mDescriptorSets.push_back(std::move(set));
            results.push_back(mDescriptorSets.back().get());
        }
        return results;
    }
    catch (vk::OutOfPoolMemoryError &)
    {
        mExhaustedPools.push_back(std::move(allocatablePool));
        mAllocatablePools.erase(std::remove_if(mAllocatablePools.begin(), mAllocatablePools.end(),
                                               [&allocatablePool](const vk::UniqueDescriptorPool &pool) {
                                                   return pool.get() == allocatablePool.get();
                                               }),
                                mAllocatablePools.end());
        return AllocateUniqueDescriptorSet(descriptorSetLayouts);
    }
    catch (vk::FragmentedPoolError &)
    {
        mExhaustedPools.push_back(std::move(allocatablePool));
        mAllocatablePools.erase(std::remove_if(mAllocatablePools.begin(), mAllocatablePools.end(),
                                               [&allocatablePool](const vk::UniqueDescriptorPool &pool) {
                                                   return pool.get() == allocatablePool.get();
                                               }),
                                mAllocatablePools.end());
        return AllocateUniqueDescriptorSet(descriptorSetLayouts);
    }
}

void DescriptorManager::ResetDescriptorPool()
{
    for (auto &exhaustedPool : mExhaustedPools)
    {
        mContext->GetDevice().resetDescriptorPool(exhaustedPool.get());
        mAllocatablePools.push_back(std::move(exhaustedPool));
    }
    mExhaustedPools.clear();
}

void DescriptorManager::UpdateUniformDescriptorSet(const std::vector<Buffer *> &uniformBuffers, uint32_t binding,
                                                   vk::DescriptorSet dstSet)
{
    std::vector<vk::DescriptorBufferInfo> descriptorBufferInfos;
    descriptorBufferInfos.reserve(uniformBuffers.size());
    for (auto uniformBuffer : uniformBuffers)
    {
        vk::DescriptorBufferInfo descriptorBufferInfo;
        descriptorBufferInfo.setBuffer(uniformBuffer->GetHandle())
            .setOffset(0)
            .setRange(uniformBuffer->GetAllocationInfo().size);
        descriptorBufferInfos.push_back(descriptorBufferInfo);
    }

    vk::WriteDescriptorSet writer;
    writer.setBufferInfo(descriptorBufferInfos)
        .setDescriptorType(vk::DescriptorType::eUniformBuffer)
        .setDescriptorCount(uniformBuffers.size())
        .setDstArrayElement(0)
        .setDstBinding(binding)
        .setDstSet(dstSet);
    mContext->GetDevice().updateDescriptorSets({writer}, {});
}

void DescriptorManager::UpdateCombinedSamplerImageDescriptorSet(std::vector<ImageDescriptor> imageDescriptors,
                                                                uint32_t binding, vk::DescriptorSet dstSet)
{
    mLogger->Debug("Updating descriptor set: handle={}", static_cast<void *>(dstSet));
    vk::WriteDescriptorSet writer;
    std::vector<vk::DescriptorImageInfo> descriptorImageInfos;
    descriptorImageInfos.reserve(imageDescriptors.size());
    for (auto &imageDescriptor : imageDescriptors)
    {
        mLogger->Debug("ImageView Handle: {}", static_cast<void *>(imageDescriptor.imageView));
        mLogger->Debug("Sampler Handle: {}", static_cast<void *>(imageDescriptor.sampler));
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
    mContext->GetDevice().updateDescriptorSets({writer}, {});
    mLogger->Debug("Combined sampler image descriptor set updated");
}

} // namespace MEngine