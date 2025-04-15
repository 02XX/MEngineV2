#pragma once
#include "Context.hpp"
#include "Interface/ILogger.hpp"
#include "MEngine.hpp"
#include "NoCopyable.hpp"
#include "PipelineLayoutManager.hpp"
#include "RenderPassManager.hpp"
#include "ShaderManager.hpp"
#include "Vertex.hpp"
#include <memory>
#include <vulkan/vulkan.hpp>

namespace MEngine
{
enum class PipelineType
{
    // 基础渲染
    ShadowMap,               // 阴影深度贴图管线
    ForwardOpaquePBR,        // 前向渲染不透明物体管线
    ForwardTransparentPBR,   // 前向渲染透明物体管线
    ForwardOpaquePhong,      // 前向渲染不透明物体管线（Phong）
    ForwardTransparentPhong, // 前向渲染透明物体管线（Phong）
    DeferredGBuffer,         // 延迟渲染 GBuffer 阶段管线
    DeferredLighting,        // 延迟渲染 Lighting 阶段管线0.

    // 屏幕空间特效
    ScreenSpaceEffectSSAO, // 屏幕空间环境光遮蔽
    ScreenSpaceEffectSSR,  // 屏幕空间反射

    // 角色与动画
    SkinnedMesh, // 骨骼动画（顶点蒙皮）
    MorphTarget, // 形变动画（顶点插值）

    // 特效与粒子
    ParticleCPU, // CPU驱动的粒子系统（动态顶点数据）
    ParticleGPU, // GPU驱动的粒子系统（计算着色器+Transform Feedback）
    Decal,       // 贴花渲染（深度测试混合）

    // 后期处理
    PostProcessToneMapping, // 色调映射
    PostProcessBloom,       // 泛光效果

    PostProcessDOF,                 // 景深效果
    PostProcessMotionBlur,          // 运动模糊
    PostProcessFXAA,                // 快速近似抗锯齿
    PostProcessSMAA,                // 自适应多重采样抗锯齿
    PostProcessVignette,            // 渐晕效果
    PostProcessChromaticAberration, // 色差效果
    PostProcessFilmGrain,           // 胶卷颗粒效果
    PostProcessColorGrading,        // 色彩分级

    // 2D与UI
    UISprite, // 精灵渲染（UI元素）
    UIText,   // 文本渲染（UI文本）

    // 特殊渲染
    Toon,     // 卡通渲染
    Wireframe // 线框渲染（仅用于调试）

    // 扩展
    // TODO: 添加更多管线类型
};
class PipelineManager final : public NoCopyable
{
  private:
    // DI
    std::shared_ptr<ILogger> mLogger;
    std::shared_ptr<Context> mContext;
    std::shared_ptr<ShaderManager> mShaderManager;
    std::shared_ptr<PipelineLayoutManager> mPipelineLayoutManager;
    std::shared_ptr<RenderPassManager> mRenderPassManager;

  private:
    std::unordered_map<PipelineType, vk::UniquePipeline> mPipelines;

  private:
    void CreateShadowMapPipeline();
    void CreateForwardOpaquePBRPipeline();
    void CreateForwardOpaquePhongPipeline();
    void CreateForwardTransparentPBRPipeline();
    void CreateForwardTransparentPhongPipeline();
    void CreateDeferredGBufferPipeline();
    void CreateDeferredLightingPipeline();
    void CreateScreenSpaceEffectSSAOPipeline();
    void CreateScreenSpaceEffectSSRPipeline();
    void CreateSkinnedMeshPipeline();
    void CreateMorphTargetPipeline();
    void CreateParticleCPUPipeline();
    void CreateParticleGPUPipeline();
    void CreateDecalPipeline();
    void CreatePostProcessToneMappingPipeline();
    void CreatePostProcessBloomPipeline();
    void CreatePostProcessDOFPipeline();
    void CreatePostProcessMotionBlurPipeline();
    void CreatePostProcessFXAAPipeline();
    void CreatePostProcessSMAAPipeline();
    void CreatePostProcessVignettePipeline();
    void CreatePostProcessChromaticAberrationPipeline();
    void CreatePostProcessFilmGrainPipeline();
    void CreatePostProcessColorGradingPipeline();
    void CreateUISpritePipeline();
    void CreateUITextPipeline();
    void CreateToonPipeline();
    void CreateWireframePipeline();

  public:
    PipelineManager(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                    std::shared_ptr<ShaderManager> shaderManager,
                    std::shared_ptr<PipelineLayoutManager> pipelineLayoutManager,
                    std::shared_ptr<RenderPassManager> renderPassManager);
    vk::Pipeline GetPipeline(PipelineType type) const;
};
} // namespace MEngine
