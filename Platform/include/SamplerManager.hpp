#pragma once
#include "Context.hpp"
#include "MEngine.hpp"
#include "SharedHandle.hpp"
#include <vulkan/vulkan.hpp>

namespace MEngine
{
using UniqueSampler = vk::UniqueSampler;
class MENGINE_API SamplerManagerManager final
{
  public:
    SamplerManagerManager() = default;
    UniqueSampler CreateUniqueSampler(vk::Filter magFilter = vk::Filter::eLinear,
                                      vk::Filter minFilter = vk::Filter::eLinear,
                                      vk::SamplerMipmapMode mipmapMode = vk::SamplerMipmapMode::eLinear,
                                      vk::SamplerAddressMode addressMode = vk::SamplerAddressMode::eRepeat,
                                      float mipLodBias = 0.0f, vk::Bool32 anisotropyEnable = vk::True,
                                      float maxAnisotropy = 16.0f, vk::Bool32 compareEnable = vk::False,
                                      vk::CompareOp compareOp = vk::CompareOp::eAlways, float minLod = 0.0f,
                                      float maxLod = vk::LodClampNone,
                                      vk::BorderColor borderColor = vk::BorderColor::eFloatOpaqueBlack,
                                      vk::Bool32 unnormalizedCoordinates = vk::False);
    SharedSampler CreateSharedSampler(vk::Filter magFilter = vk::Filter::eLinear,
                                      vk::Filter minFilter = vk::Filter::eLinear,
                                      vk::SamplerMipmapMode mipmapMode = vk::SamplerMipmapMode::eLinear,
                                      vk::SamplerAddressMode addressMode = vk::SamplerAddressMode::eRepeat,
                                      float mipLodBias = 0.0f, vk::Bool32 anisotropyEnable = vk::True,
                                      float maxAnisotropy = 16.0f, vk::Bool32 compareEnable = vk::False,
                                      vk::CompareOp compareOp = vk::CompareOp::eAlways, float minLod = 0.0f,
                                      float maxLod = vk::LodClampNone,
                                      vk::BorderColor borderColor = vk::BorderColor::eFloatOpaqueBlack,
                                      vk::Bool32 unnormalizedCoordinates = vk::False);
};
} // namespace MEngine