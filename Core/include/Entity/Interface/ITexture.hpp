#pragma once
#include "IEntity.hpp"
#include <vulkan/vulkan.hpp>
using namespace boost::uuids;
namespace MEngine
{
class ITexture : public virtual IEntity<UUID>
{
  protected:
  public:
    virtual ~ITexture() = default;
    // Getters
    virtual vk::Image GetImage() const = 0;
    virtual vk::ImageView GetImageView() const = 0;
    virtual vk::Sampler GetSampler() const = 0;
    virtual const std::filesystem::path &GetImagePath() const = 0;
    // Setters
    virtual void SetImagePath(const std::filesystem::path &path) = 0;
};
} // namespace MEngine

namespace nlohmann
{
template <> struct adl_serializer<MEngine::ITexture>
{
    static void to_json(json &j, const MEngine::ITexture &material)
    {
        auto &entity = static_cast<const MEngine::IEntity<MEngine::UUID> &>(material);
        j = entity;
    }
    static void from_json(const json &j, MEngine::ITexture &material)
    {
        auto &entity = static_cast<MEngine::IEntity<MEngine::UUID> &>(material);
        j.get_to(entity);
    }
};
} // namespace nlohmann