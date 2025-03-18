#pragma once

#include "Context.hpp"
#include "Logger.hpp"
#include "MEngine.hpp"
#include <vulkan/vulkan.hpp>
namespace MEngine
{
// 描述符绑定信息
struct DescriptorBindingInfo
{
    uint32_t binding;
    vk::DescriptorType type;
    uint32_t count;
    vk::ShaderStageFlags stageFlags;
};
using SharedPipelineLayout = std::shared_ptr<vk::PipelineLayout>;
class MENGINE_API PipelineLayoutManager final
{
  public:
    vk::UniquePipelineLayout CreateUniquePipelineLayout(const std::vector<DescriptorBindingInfo> &descriptorBindings,
                                                        const std::vector<vk::PushConstantRange> &pushConstants);
    SharedPipelineLayout CreateSharedPipelineLayout(const std::vector<DescriptorBindingInfo> &descriptorBindings,
                                                    const std::vector<vk::PushConstantRange> &pushConstants);
};

} // namespace MEngine