#pragma once

#include "IEntity.hpp"
#include "NoCopyable.hpp"
#include "PipelineLayoutManager.hpp"
#include "PipelineManager.hpp"

#include "boost/uuid.hpp"
#include "boost/uuid/uuid.hpp"
#include <cstdint>
#include <vulkan/vulkan_handles.hpp>
using namespace boost::uuids;
namespace MEngine
{
class IMaterialMetadata
{
  public:
    uuid ID = random_generator()();
    std::string materialName = "DefaultMaterial";
    PipelineLayoutType pipelineType = PipelineLayoutType::OpaquePBR;
};
class IMaterial : public IEntity
{
  protected:
  public:
    virtual ~IMaterial() = default;
    // Getters
    virtual PipelineLayoutType GetPipelineType() const = 0;
    virtual vk::DescriptorSet GetDescriptorSet() const = 0;
    // Setters
    virtual void SetPipelineType(PipelineLayoutType type) = 0;
};
} // namespace MEngine