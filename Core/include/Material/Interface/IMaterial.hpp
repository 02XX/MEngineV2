#pragma once

#include "NoCopyable.hpp"
#include "PipelineManager.hpp"
#include "Texture.hpp"
#include <cstdint>
namespace MEngine
{
class IMaterial : public NoCopyable
{
  protected:
  public:
    virtual ~IMaterial() = default;
    virtual PipelineType GetPipelineType() const = 0;
    virtual void SetPipelineType(PipelineType type) = 0;
    virtual std::string GetMaterialName() const = 0;
    virtual uint32_t GetMaterialID() const = 0;
    virtual vk::DescriptorSet GetMaterialDescriptorSet() const = 0;
    virtual void Update() = 0;
};
} // namespace MEngine