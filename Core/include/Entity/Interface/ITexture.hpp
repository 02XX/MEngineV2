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
class ITextureMetadata : IMetadata
{
  public:
    std::filesystem::path imagePath{};
};
class ITexture : public IEntity
{
  protected:
  public:
    virtual ~ITexture() = default;
    // Getters
    virtual vk::Image GetImage() const = 0;
    virtual vk::ImageView GetImageView() const = 0;
    virtual vk::Sampler GetSampler() const = 0;
    virtual std::filesystem::path GetImagePath() const = 0;
    // Setters
    virtual void SetImagePath(const std::filesystem::path &path) = 0;
};
} // namespace MEngine