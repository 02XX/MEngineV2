#pragma once

#include "Context.hpp"
#include "Image.hpp"
#include "ImageFactory.hpp"
#include "Interface/IConfigure.hpp"
#include "Interface/ILogger.hpp"
#include "MEngine.hpp"
#include "NoCopyable.hpp"

#include "SDLWindow.hpp"
#include "magic_enum/magic_enum.hpp"
#include <cstdint>
#include <functional>
#include <memory>
#include <span>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.hpp>
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
    ForwardComposition,  // Forward, 前向渲染subpass0: 不透明物体渲染 subpass1: 透明物体渲染，
                         // 创建多个MRT，Phong只渲染第一个MRT，PBR渲染所有MRT
    Sky,                 // 天空盒渲染subpass0: 天空盒渲染
    Transparent,         // Forward 透明物体渲染subpass0: 透明物体渲染
    PostProcess,         // 后处理渲染subpass0: 后处理渲染
    UI,                  // UI渲染subpass0: UI渲染
    EditorUI,            // 编辑器UI渲染subpass0: UI渲染
};
struct RenderTarget
{
    // Render target 0: Color
    UniqueImage colorImage;
    vk::UniqueImageView colorImageView;
    // Render target 1: Depth/Stencil
    UniqueImage depthStencilImage;
    vk::UniqueImageView depthStencilImageView;
    // Render target 2: Albedo
    UniqueImage albedoImage;
    vk::UniqueImageView albedoImageView;
    // Render target 3: Normal
    UniqueImage normalImage;
    vk::UniqueImageView normalImageView;
    // Render target 4: WorldPos
    UniqueImage worldPosImage;
    vk::UniqueImageView worldPosImageView;
    // Render target 5: MetallicRoughness
    UniqueImage metallicRoughnessImage;
    vk::UniqueImageView metallicRoughnessImageView;
    // Render target 6: Emissive
    UniqueImage emissiveImage;
    vk::UniqueImageView emissiveImageView;
};
struct EditorRenderTarget
{
    // Editor target 0: Color
    UniqueImage colorImage;
    vk::UniqueImageView colorImageView;
};
class RenderPassManager final : public NoCopyable
{
  private:
    // DI
    std::shared_ptr<ILogger> mLogger;
    std::shared_ptr<Context> mContext;
    std::shared_ptr<ImageFactory> mImageFactory;
    std::shared_ptr<IConfigure> mConfigure;

  private:
    std::unordered_map<RenderPassType, vk::UniqueRenderPass> mRenderPasses;
    std::unordered_map<RenderPassType, std::vector<vk::UniqueFramebuffer>> mFrameBuffers;
    uint32_t mRenderTargetWidth;
    uint32_t mRenderTargetHeight;
    uint32_t mEditorRenderTargetWidth;
    uint32_t mEditorRenderTargetHeight;

  private:
    std::vector<std::unique_ptr<RenderTarget>> mRenderTargets;
    std::vector<std::unique_ptr<EditorRenderTarget>> mEditorRenderTargets;

  private:
    void CreateShadowDepthRenderPass();
    void CreateDeferredCompositionRenderPass();
    void CreateForwardCompositionRenderPass();
    void CreateSkyRenderPass();
    void CreateTransparentRenderPass();
    void CreatePostProcessRenderPass();
    void CreateUIRenderPass();
    void CreateEditorUIRenderPass();

    void CreateRenderTarget();
    void CreateEditorRenderTarget();

    void CreateShadowDepthFrameBuffer();
    void CreateDeferredCompositionFrameBuffer();
    void CreateForwardCompositionFrameBuffer();
    void CreateSkyFrameBuffer();
    void CreateTransparentFrameBuffer();
    void CreatePostProcessFrameBuffer();
    void CreateUIFrameBuffer();
    void CreateEditorUIFrameBuffer();

  public:
    RenderPassManager(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                      std::shared_ptr<IConfigure> configure, std::shared_ptr<ImageFactory> imageFactory);
    vk::RenderPass GetRenderPass(RenderPassType type) const;
    std::vector<vk::Framebuffer> GetFrameBuffer(RenderPassType type) const;
    void RecreateRenderTargetFrameBuffer(uint32_t width, uint32_t height);
    void RecreateEditorRenderTargetFrameBuffer(uint32_t width, uint32_t height);
    vk::Extent2D GetRenderTargetExtent() const
    {
        return vk::Extent2D{mRenderTargetWidth, mRenderTargetHeight};
    }
    vk::Extent2D GetEditorRenderTargetExtent() const
    {
        return vk::Extent2D{mEditorRenderTargetWidth, mEditorRenderTargetHeight};
    }

  public:
    inline auto GetRenderTargets() const
    {
        return mRenderTargets |
               std::views::transform(
                   [](const std::unique_ptr<RenderTarget> &ptr) -> const RenderTarget & { return *ptr; });
    }
    inline auto GetEditorRenderTargets() const
    {
        return mEditorRenderTargets |
               std::views::transform(
                   [](const std::unique_ptr<EditorRenderTarget> &ptr) -> const EditorRenderTarget & { return *ptr; });
    }
};

} // namespace MEngine