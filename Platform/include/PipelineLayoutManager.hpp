#pragma once
#include "Context.hpp"
#include "Logger.hpp"
#include "MEngine.hpp"
#include "SharedHandle.hpp"
#include <memory>
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
using UniquePipelineLayout = vk::UniquePipelineLayout;
class MENGINE_API PipelineLayoutManager final
{
  public:
    UniquePipelineLayout CreateUniquePipelineLayout(const std::vector<DescriptorBindingInfo> &descriptorBindings,
                                                    const std::vector<vk::PushConstantRange> &pushConstants);
    SharedPipelineLayout CreateSharedPipelineLayout(const std::vector<DescriptorBindingInfo> &descriptorBindings,
                                                    const std::vector<vk::PushConstantRange> &pushConstants);
};

} // namespace MEngine