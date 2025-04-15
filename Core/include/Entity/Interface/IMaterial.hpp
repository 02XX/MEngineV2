#pragma once

#include "IEntity.hpp"
#include "NoCopyable.hpp"
#include "PipelineLayoutManager.hpp"
#include "PipelineManager.hpp"

#include "RenderPassManager.hpp"
#include "boost/uuid.hpp"
#include <cstdint>
#include <vulkan/vulkan_handles.hpp>
namespace MEngine
{
using UUID = boost::uuids::uuid;
enum class RenderType
{
    ForwardOpaquePBR, // RenderPass: ForwardComposition, subpass0, PipelineLayout: PBR, PipelineType: ForwardOpaquePBR
    ForwardTransparentPBR,   // RenderPass: ForwardComposition, subpass1, PipelineLayout: PBR, PipelineType:
                             // ForwardTransparentPBR
    ForwardOpaquePhong,      // RenderPass: ForwardComposition, subpass0, PipelineLayout: Phong, PipelineType:
                             // ForwardOpaquePhong
    ForwardTransparentPhong, // RenderPass: ForwardComposition, subpass1, PipelineLayout: Phong, PipelineType:
                             // ForwardTransparentPhong
    Deferred, // RenderPass: DeferredComposition,subpass0, subpass1,  PipelineLayout: PBR, PipelineType: DeferredGBuffer
};
class IMaterial : public virtual IEntity<UUID>
{
  protected:
  public:
    virtual ~IMaterial() = default;
    // Getters
    virtual RenderType GetRenderType() const = 0;
    // Vulkan Resources
    virtual vk::DescriptorSet GetDescriptorSet() const = 0;
};
} // namespace MEngine

namespace nlohmann
{
template <> struct adl_serializer<MEngine::IMaterial>
{
    static void to_json(json &j, const MEngine::IMaterial &material)
    {
        auto &entity = static_cast<const MEngine::IEntity<MEngine::UUID> &>(material);
        j = entity;
    }
    static void from_json(const json &j, MEngine::IMaterial &material)
    {
        auto &entity = static_cast<MEngine::IEntity<MEngine::UUID> &>(material);
        j.get_to(entity);
    }
};
} // namespace nlohmann