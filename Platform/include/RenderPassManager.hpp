#pragma once

#include "Context.hpp"
#include "Image.hpp"
#include "ImageManager.hpp"
#include "Logger.hpp"
#include "MEngine.hpp"
#include "NoCopyable.hpp"
#include "magic_enum/magic_enum.hpp"
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.hpp>

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
    vk::Image swapchainImage;
    vk::ImageView swapchainImageView;
};
struct UIFrameResource
{
    // 1. swapchain image
    vk::Image swapchainImage;
    vk::ImageView swapchainImageView;
};
struct TranslucencyFrameResource
{
    // 1. swapchain image
    vk::Image swapchainImage;
    vk::ImageView swapchainImageView;
    // 2. depth stencil
    UniqueImage depthStencilImage;
    vk::UniqueImageView depthStencilImageView;
};
class RenderPassManager final : public NoCopyable
{
  private:
    std::shared_ptr<ImageManager> mImageManager;

  private:
    std::unordered_map<RenderPassType, vk::UniqueRenderPass> mRenderPasses;
    std::unordered_map<RenderPassType, std::vector<vk::UniqueFramebuffer>> mFrameBuffers;
    std::vector<DefferFrameResource> mDefferFrameResources;
    std::vector<UIFrameResource> mUIFrameResources;
    std::vector<TranslucencyFrameResource> mTranslucencyFrameResources;

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
    const DefferFrameResource &GetDefferFrameResource(uint32_t index) const
    {
        return mDefferFrameResources[index];
    }
    const UIFrameResource &GetUIFrameResource(uint32_t index) const
    {
        return mUIFrameResources[index];
    }
    const TranslucencyFrameResource &GetTranslucencyFrameResource(uint32_t index) const
    {
        return mTranslucencyFrameResources[index];
    }
};

} // namespace MEngine