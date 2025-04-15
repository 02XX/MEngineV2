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
#include <vulkan/vulkan_structs.hpp>
struct RenderSetting
{
    // "Render": {
    //     "Resolution": {
    //         "Width": 1920,
    //         "Height": 1080
    //     }
    // },
};
namespace MEngine
{
enum class RenderPassType
{
    ShadowDepth,         // 生成所有光源的阴影贴图subpass0: 生成阴影贴图
    DeferredComposition, // Deferred, 延迟渲染Subpass0: GBuffer, Subpass1: Lighting
    ForwardComposition,  // Forward, 前向渲染subpass0: 不透明物体渲染 subpass1: 透明物体渲染
    Sky,                 // 天空盒渲染subpass0: 天空盒渲染
    Transparent,         // Forward 透明物体渲染subpass0: 透明物体渲染
    PostProcess,         // 后处理渲染subpass0: 后处理渲染
    UI,                  // UI渲染subpass0: UI渲染
};
struct TransparentFrameResource
{
    // Render Target
    vk::UniqueImageView renderTargetImageView;
    UniqueImage renderTargetImage;
    // Depth Stencil
    vk::UniqueImageView depthStencilImageView;
    UniqueImage depthStencilImage;
};
struct UIFrameResource
{
    // Render Target
    vk::ImageView renderTargetImageView; // Swapchain Image View
    vk::Image renderTargetImage;         // Swapchain Image
    // Depth Stencil
    vk::UniqueImageView depthStencilImageView;
    UniqueImage depthStencilImage;
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
    uint32_t mWidth = 800;
    uint32_t mHeight = 600;

  private:
    std::vector<std::shared_ptr<TransparentFrameResource>> mTransparentFrameResources;
    std::vector<std::shared_ptr<UIFrameResource>> mUIFrameResources;

  private:
    void CreateShadowDepthRenderPass();
    void CreateDeferredCompositionRenderPass();
    void CreateForwardCompositionRenderPass();
    void CreateSkyRenderPass();
    void CreateTransparentRenderPass();
    void CreatePostProcessRenderPass();
    void CreateUIRenderPass();

    void CreateShadowDepthFrameBuffer();
    void CreateDeferredCompositionFrameBuffer();
    void CreateForwardCompositionFrameBuffer();
    void CreateSkyFrameBuffer();
    void CreateTransparentFrameBuffer();
    void CreatePostProcessFrameBuffer();
    void CreateUIFrameBuffer();

  public:
    RenderPassManager(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                      std::shared_ptr<ImageFactory> imageFactory);
    vk::RenderPass GetRenderPass(RenderPassType type) const;
    std::vector<vk::Framebuffer> GetFrameBuffer(RenderPassType type) const;
    void RecreateUIFrameBuffer();
    void RecreateFrameBuffer(uint32_t width, uint32_t height);
    vk::Extent2D GetExtent() const
    {
        return vk::Extent2D(mWidth, mHeight);
    }

  public:
    inline std::vector<std::shared_ptr<TransparentFrameResource>> &GetTransparentFrameResource()
    {
        return mTransparentFrameResources;
    }
    inline std::vector<std::shared_ptr<UIFrameResource>> &GetUIFrameResource()
    {
        return mUIFrameResources;
    }
};

} // namespace MEngine