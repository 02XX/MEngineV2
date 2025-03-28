#pragma once
#include "Context.hpp"
#include "MEngine.hpp"
#include "NoCopyable.hpp"
#include <vulkan/vulkan.hpp>

namespace MEngine
{
class FrameBufferManager final : public NoCopyable
{
  private:
    vk::UniqueFramebuffer mFramebuffer;

  public:
    FrameBufferManager(vk::RenderPass renderPass, vk::Extent2D extent, const std::vector<vk::ImageView> &attachments);
    vk::Framebuffer GetFramebuffer() const;
};
} // namespace MEngine