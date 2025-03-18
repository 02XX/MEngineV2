#pragma once
#include "Context.hpp"
#include "MEngine.hpp"
#include <vulkan/vulkan.hpp>

namespace MEngine
{
class MENGINE_API FrameBufferManager final
{
  public:
    FrameBufferManager();
    vk::UniqueFramebuffer CreateFrameBuffer(vk::RenderPass renderPass, vk::Extent2D extent,
                                            const std::vector<vk::ImageView> &attachments);
};
} // namespace MEngine