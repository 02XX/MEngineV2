#include "PipelineLayoutManager.hpp"

namespace MEngine
{
UniquePipelineLayout PipelineLayoutManager::CreateUniquePipelineLayout(
    const std::vector<DescriptorBindingInfo> &descriptorBindings,
    const std::vector<vk::PushConstantRange> &pushConstants)
{
    auto &context = Context::Instance();
    vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding;
    std::vector<vk::DescriptorSetLayoutBinding> descriptorSetLayoutBindings;
    for (const auto &descriptorBinding : descriptorBindings)
    {
        descriptorSetLayoutBinding.setBinding(descriptorBinding.binding)
            .setDescriptorType(descriptorBinding.type)
            .setDescriptorCount(descriptorBinding.count)
            .setStageFlags(descriptorBinding.stageFlags);
        descriptorSetLayoutBindings.push_back(descriptorSetLayoutBinding);
    }
    vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;
    descriptorSetLayoutCreateInfo.setBindings(descriptorSetLayoutBindings);

    vk::UniqueDescriptorSetLayout descriptorSetLayout =
        context.GetDevice()->createDescriptorSetLayoutUnique(descriptorSetLayoutCreateInfo);

    pipelineLayoutCreateInfo.setSetLayouts(descriptorSetLayout.get()).setPushConstantRanges(pushConstants);
    auto pipelineLayout = context.GetDevice()->createPipelineLayoutUnique(pipelineLayoutCreateInfo);
    LogI("Graphic Pipeline Layout Created.");
    return std::move(pipelineLayout);
}
SharedPipelineLayout PipelineLayoutManager::CreateSharedPipelineLayout(
    const std::vector<DescriptorBindingInfo> &descriptorBindings,
    const std::vector<vk::PushConstantRange> &pushConstants)
{
    auto &context = Context::Instance();
    auto uniquePipelineLayout = CreateUniquePipelineLayout(descriptorBindings, pushConstants);
    vk::PipelineLayout pipelineLayout = uniquePipelineLayout.release();
    SharedPipelineLayout sharedPipelineLayout(pipelineLayout);
    return sharedPipelineLayout;
}
} // namespace MEngine