#pragma once
#include "Context.hpp"
#include "MEngine.hpp"
#include <vulkan/vulkan.hpp>

namespace MEngine
{
class MENGINE_API FrameBufferManager final
{
  private:
    vk::UniqueFramebuffer mFramebuffer;

  public:
    FrameBufferManager(vk::RenderPass renderPass, vk::Extent2D extent, const std::vector<vk::ImageView> &attachments);
    vk::Framebuffer GetFramebuffer() const;
};
} // namespace MEngine