#pragma once
#include "Buffer.hpp"
#include "Context.hpp"
#include "Interface/IConfigure.hpp"
#include "Interface/ILogger.hpp"
#include "MEngine.hpp"
#include "NoCopyable.hpp"
#include <functional>
#include <memory>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace nlohmann
{
template <> struct adl_serializer<std::pair<vk::DescriptorType, float>>
{
    static void to_json(json &j, const std::pair<vk::DescriptorType, float> &p)
    {
        j = json{{"type", vk::to_string(p.first)}, {"value", p.second}};
    }

    static void from_json(const json &j, std::pair<vk::DescriptorType, float> &p)
    {
        static const auto type_map = std::unordered_map<std::string, vk::DescriptorType>{
            {"eSampler", vk::DescriptorType::eSampler},
            {"eCombinedImageSampler", vk::DescriptorType::eCombinedImageSampler},
            {"eSampledImage", vk::DescriptorType::eSampledImage},
            {"eStorageImage", vk::DescriptorType::eStorageImage},
            {"eUniformBuffer", vk::DescriptorType::eUniformBuffer},
            {"eStorageBuffer", vk::DescriptorType::eStorageBuffer},
            {"eUniformBufferDynamic", vk::DescriptorType::eUniformBufferDynamic},
            {"eStorageBufferDynamic", vk::DescriptorType::eStorageBufferDynamic}};

        const auto type_str = j.at("type").get<std::string>();
        p.first = type_map.at(type_str);
        p.second = j.at("value").get<float>();
    }
};
} // namespace nlohmann

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
    std::shared_ptr<IConfigure> mConfigure;

  private:
    PoolSizesProportion mDefaultPoolSizesProportion;
    uint32_t mMaxDescriptorSize;
    std::vector<vk::UniqueDescriptorPool> mExhaustedPools;
    std::vector<vk::UniqueDescriptorPool> mAllocatablePools;

    vk::UniqueDescriptorPool &AcquireAllocatablePool();

  public:
    DescriptorManager(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                      std::shared_ptr<IConfigure> configure);

    std::vector<UniqueDescriptorSet> AllocateUniqueDescriptorSet(
        const std::vector<vk::DescriptorSetLayout> &descriptorSetLayouts);
    void ResetDescriptorPool();
    void SetDefaultPoolSizesProportion(PoolSizesProportion defaultPoolSizesProportion)
    {
        mDefaultPoolSizesProportion = defaultPoolSizesProportion;
    }
    void UpdateUniformDescriptorSet(const std::vector<std::reference_wrapper<Buffer>> &uniformBuffers, uint32_t binding,
                                    vk::DescriptorSet dstSet);
    void UpdateCombinedSamplerImageDescriptorSet(std::vector<ImageDescriptor> imageDescriptors, uint32_t binding,
                                                 vk::DescriptorSet dstSet);
};

} // namespace MEngine