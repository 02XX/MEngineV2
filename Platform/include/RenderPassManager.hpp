#pragma once

#include "Context.hpp"
#include "Image.hpp"
#include "ImageFactory.hpp"
#include "Interface/ILogger.hpp"
#include "MEngine.hpp"
#include "NoCopyable.hpp"

#include "magic_enum/magic_enum.hpp"
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
    // // 7. swapchain image
    // vk::Image swapchainImage;
    // vk::ImageView swapchainImageView;
    // 7. colour attachment
    UniqueImage renderImage;
    vk::UniqueImageView renderImageView;
};
struct UIFrameResource
{
    // 1. swapchain image
    vk::Image swapchainImage;
    vk::ImageView swapchainImageView;
};
struct TranslucencyFrameResource
{
    // 1. colour attachment
    UniqueImage renderImage;
    vk::UniqueImageView renderImageView;
    // 2. depth stencil
    UniqueImage depthStencilImage;
    vk::UniqueImageView depthStencilImageView;
};
class RenderPassManager final : public NoCopyable
{
  private:
    // DI
    std::shared_ptr<ILogger> mLogger;
    std::shared_ptr<Context> mContext;
    std::shared_ptr<ImageFactory> mImageFactory;

  private:
    std::unordered_map<RenderPassType, vk::UniqueRenderPass> mRenderPasses;
    std::unordered_map<RenderPassType, std::vector<vk::UniqueFramebuffer>> mFrameBuffers;
    std::vector<std::shared_ptr<DefferFrameResource>> mDefferFrameResources;
    std::vector<std::shared_ptr<UIFrameResource>> mUIFrameResources;
    std::vector<std::shared_ptr<TranslucencyFrameResource>> mTranslucencyFrameResources;

    uint32_t mWidth = 800;
    uint32_t mHeight = 600;

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
    RenderPassManager(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                      std::shared_ptr<ImageFactory> imageFactory);
    vk::RenderPass GetRenderPass(RenderPassType type) const;
    std::vector<vk::Framebuffer> GetFrameBuffer(RenderPassType type) const;

    inline const std::vector<std::shared_ptr<DefferFrameResource>> &GetDefferFrameResource() const
    {
        return mDefferFrameResources;
    }
    inline const std::vector<std::shared_ptr<UIFrameResource>> &GetUIFrameResource() const
    {
        return mUIFrameResources;
    }
    inline const std::vector<std::shared_ptr<TranslucencyFrameResource>> &GetTranslucencyFrameResource() const
    {
        return mTranslucencyFrameResources;
    }
    void RecreateFrameBuffer(uint32_t width, uint32_t height);
    vk::Extent2D GetExtent() const
    {
        return vk::Extent2D(mWidth, mHeight);
    }
};

} // namespace MEngine