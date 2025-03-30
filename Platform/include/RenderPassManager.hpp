#pragma once

#include "Context.hpp"
#include "Image.hpp"
#include "ImageManager.hpp"
#include "Logger.hpp"
#include "MEngine.hpp"
#include "NoCopyable.hpp"
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace MEngine
{
enum class RenderPassType
{
    Forward,      // 前向渲染
    Deffer,       // 延迟渲染
    ShadowDepth,  // 阴影深度图渲染
    Translucency, // 半透明物体渲染
    PostProcess,  // 后处理特效
    Sky,          // 天空盒/大气渲染
    UI            // 界面渲染
};
struct DefferFrameResource
{
    // 1. world space position
    UniqueImage positionImage;
    vk::UniqueImageView positionImageView;
    // 2. normal
    UniqueImage normalImage;
    vk::UniqueImageView normalImageView;
    // 3. albedo
    UniqueImage albedoImage;
    vk::UniqueImageView albedoImageView;
    // 4. metal rough
    UniqueImage metalRoughImage;
    vk::UniqueImageView metalRoughImageView;
    // 5. ao
    UniqueImage aoImage;
    vk::UniqueImageView aoImageView;
    // 6. depth stencil
    UniqueImage depthStencilImage;
    vk::UniqueImageView depthStencilImageView;
    // 7. swapchain image
    vk::ImageView swapchainImageView;
};
class RenderPassManager final : public NoCopyable
{
  private:
    std::shared_ptr<ImageManager> mImageManager;

  private:
    std::unordered_map<RenderPassType, vk::UniqueRenderPass> mRenderPasses;
    std::unordered_map<RenderPassType, std::vector<vk::UniqueFramebuffer>> mFrameBuffers;
    std::vector<DefferFrameResource> mDefferFrameResources;

  private:
    void CreateGBufferRenderPass();
    void CreateShadowDepthRenderPass();
    void CreateLightingRenderPass();
    void CreateTranslucencyRenderPass();
    void CreatePostProcessRenderPass();
    void CreateSkyRenderPass();
    void CreateUIRenderPass();

    void CreateDefferFrameBuffer();
    void CreateShadowDepthFrameBuffer();
    void CreateLightingFrameBuffer();
    void CreateTranslucencyFrameBuffer();
    void CreatePostProcessFrameBuffer();
    void CreateSkyFrameBuffer();
    void CreateUIFrameBuffer();

  public:
    RenderPassManager(std::shared_ptr<ImageManager> imageManager);
    vk::RenderPass GetRenderPass(RenderPassType type) const;
    vk::Framebuffer GetFrameBuffer(RenderPassType type, uint32_t index) const;
};

} // namespace MEngine