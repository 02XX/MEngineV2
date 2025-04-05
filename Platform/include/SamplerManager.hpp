#pragma once
#include "Context.hpp"
#include "MEngine.hpp"
#include "NoCopyable.hpp"

#include <vulkan/vulkan.hpp>

namespace MEngine
{
using UniqueSampler = vk::UniqueSampler;
class SamplerManager final : public NoCopyable
{
  private:
    // DI
    std::shared_ptr<ILogger> mLogger;
    std::shared_ptr<Context> mContext;

  public:
    SamplerManager(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context);
    UniqueSampler CreateUniqueSampler(vk::Filter magFilter = vk::Filter::eLinear,
                                      vk::Filter minFilter = vk::Filter::eLinear,
                                      vk::SamplerMipmapMode mipmapMode = vk::SamplerMipmapMode::eLinear,
                                      vk::SamplerAddressMode addressMode = vk::SamplerAddressMode::eRepeat,
                                      float mipLodBias = 0.0f, vk::Bool32 anisotropyEnable = vk::False,
                                      float maxAnisotropy = 16.0f, vk::Bool32 compareEnable = vk::False,
                                      vk::CompareOp compareOp = vk::CompareOp::eAlways, float minLod = 0.0f,
                                      float maxLod = 0.0f,
                                      vk::BorderColor borderColor = vk::BorderColor::eFloatOpaqueBlack,
                                      vk::Bool32 unnormalizedCoordinates = vk::False);
};
} // namespace MEngine