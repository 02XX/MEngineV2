#pragma once
#include "Context.hpp"
#include "Interface/ILogger.hpp"
#include "MEngine.hpp"
#include "NoCopyable.hpp"
#include "glm/glm.hpp"
#include <memory>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace MEngine
{
enum class PipelineLayoutType
{
    ShadowDepth, // 阴影 深度贴图 Set0:{Camera_UBO, Light_SBO[6],
                 // ShadowParameters_SBO,ShadowMap[6](需要和Light_SBO按顺序一一对应)}
    Opaque,      // 不透明物体 Set0:{Camera_UBO, Light_SBO[6], ShadowParameters_SBO,
                 // ShadowMap[6](需要和Light_SBO按顺序一一对应)} , Set1: PBR{Parameters_UBO{Albedo, Normal,
    // MetallicRoughness, AmbientOcclusion,Emissive}, AlbedoMap, NormalMap, MetallicRoughnessMap,
    // AmbientOcclusionMap, EmissiveMap}
    Transparent, //  透明物体 Set0:{Camera_UBO, Light_SBO[6],
                 //  ShadowParameters_SBO,ShadowMap[6](需要和Light_SBO按顺序一一对应)}  , Set1: PBR{Albedo, Normal,
                 //  MetallicRoughness,
    //  AmbientOcclusion, Emissive} + AlphaBlend
    ScreenSpaceEffect, // 屏幕空间特效 Set0:{Camera_UBO, Light_SBO[6],
                       // ShadowParameters_SBO,ShadowMap[6](需要和Light_SBO按顺序一一对应)}  , Set1:
                       // PBR{Parameters_UBO{Albedo, Normal,
    Skybox, // 天空盒 Set0:{Camera_UBO, Light_SBO[6], ShadowParameters_SBO,ShadowMap[6](需要和Light_SBO按顺序一一对应)}
            // , Set1: Skybox{CubeMap, Rotation_UBO}
    Particle, // 粒子 Set0:{Camera_UBO, Light_SBO[6], ShadowParameters_SBO,ShadowMap[6](需要和Light_SBO按顺序一一对应)}
              // , Set1: Particle{ParticleAttributes_SBO, ParticleMap}
    Terrain, // 地形 Set0:{Camera_UBO, Light_SBO[6], ShadowParameters_SBO,ShadowMap[6](需要和Light_SBO按顺序一一对应)}
             // , Set1: Terrain{TerrainMap0, TerrainMap1, TerrainMap2,
    // TerrainMap3, HeightMap, TerrainAttributes_UBO{Proportion, HeightScale, ...}}
    SkinnedMesh, // 骨骼动画 Set0:{Camera_UBO, Light_SBO[6],
                 // ShadowParameters_SBO,ShadowMap[6](需要和Light_SBO按顺序一一对应)}  , Set1:
                 // PBR{Parameters_UBO{Albedo, Normal,
    // MetallicRoughness,AmbientOcclusion,Emissive}, AlbedoMap, NormalMap, MetallicRoughnessMap,
    // AmbientOcclusionMap,EmissiveMap} Set2: SkinnedMesh{BoneMatrices_SBO}
    PostProcess, // 后处理 Set0:{RenderTexture, DepthTexture, PostProcessParameters_UBO}
    UI,          // UI Set0:{UIElementMap, UIParameters_UBO{Color, Transparency...}}
    Sprite,      // 精灵 Set0:{SpriteTexture, SpriteParameters_UBO{UVOffset, SpriteIndex, ...}}
    Toon, // 卡通渲染 Set0:{Camera_UBO, Light_SBO[6], ShadowParameters_SBO,ShadowMap[6](需要和Light_SBO按顺序一一对应)}
          // , Set1: Toon{RampMap, ToonParameters_UBO{RampScale, Color,...}}
    SubsurfaceScatter, // TODO: 集成到 PBR 材质中
};

class PipelineLayoutManager final : public NoCopyable
{
  private:
    // DI
    std::shared_ptr<ILogger> mLogger;
    std::shared_ptr<Context> mContext;

  private:
    struct GlobalLayoutBindings
    {
        // Set: 0, Binding: 0 Camera
        vk::DescriptorSetLayoutBinding mCameraBinding{0, vk::DescriptorType::eUniformBuffer, 1,
                                                      vk::ShaderStageFlagBits::eVertex};
        // Set: 0, Binding: 1 Light
        vk::DescriptorSetLayoutBinding mLightBinding{1, vk::DescriptorType::eStorageBuffer, 6,
                                                     vk::ShaderStageFlagBits::eFragment};
        // Set: 0, Binding: 2 Shadow Parameters
        vk::DescriptorSetLayoutBinding mShadowParametersBinding{2, vk::DescriptorType::eStorageBuffer, 1,
                                                                vk::ShaderStageFlagBits::eFragment};
        // Set: 0, Binding: 3 Shadow Maps
        vk::DescriptorSetLayoutBinding mShadowMapsBinding{3, vk::DescriptorType::eStorageBuffer, 6,
                                                          vk::ShaderStageFlagBits::eFragment};
    } mGlobalDescriptorLayoutBindings;
    struct PBRLayoutBindings
    {
        // Set: 1, Binding: 0 Parameters
        vk::DescriptorSetLayoutBinding mParameterBinding{0, vk::DescriptorType::eUniformBuffer, 1,
                                                         vk::ShaderStageFlagBits::eFragment};
        // Set: 1, Binding: 1 Base Color
        vk::DescriptorSetLayoutBinding mBaseColorBinding{1, vk::DescriptorType::eCombinedImageSampler, 1,
                                                         vk::ShaderStageFlagBits::eFragment};
        // Set: 1, Binding: 2 Normal Map
        vk::DescriptorSetLayoutBinding mNormalMapBinding{2, vk::DescriptorType::eCombinedImageSampler, 1,
                                                         vk::ShaderStageFlagBits::eFragment};
        // Set: 1, Binding: 3 MetallicRoughness Map
        vk::DescriptorSetLayoutBinding mMetallicRoughnessBinding{3, vk::DescriptorType::eCombinedImageSampler, 1,
                                                                 vk::ShaderStageFlagBits::eFragment};
        // Set: 1, Binding: 4 Ambient Occlusion Map
        vk::DescriptorSetLayoutBinding mAmbientOcclusionBinding{4, vk::DescriptorType::eCombinedImageSampler, 1,
                                                                vk::ShaderStageFlagBits::eFragment};
        // Set: 1, Binding: 5 Emissive Map
        vk::DescriptorSetLayoutBinding mEmissiveBinding{5, vk::DescriptorType::eCombinedImageSampler, 1,
                                                        vk::ShaderStageFlagBits::eFragment};
    } mPBRDescriptorLayoutBindings;

  private:
    std::unordered_map<PipelineLayoutType, vk::UniquePipelineLayout> mPipelineLayouts;
    vk::UniqueDescriptorSetLayout mPBRDescriptorSetLayout;
    vk::UniqueDescriptorSetLayout mGlobalDescriptorSetLayout;

    // DescriptorSetLayout
    void CreateGlobalDescriptorSetLayout();
    void CreatePBRDescriptorSetLayout();

    // PipelineLayout
    void CreateShadowDepthPipelineLayout();
    void CreateOpaquePipelineLayout();
    void CreateTranslucencyPipelineLayout();
    void CreateScreenSpaceEffectPipelineLayout();
    void CreateSkyPipelineLayout();
    void CreateParticlePipelineLayout();
    void CreateTerrainPipelineLayout();
    void CreateSkinnedMeshPipelineLayout();
    void CreatePostProcessPipelineLayout();
    void CreateUIPipelineLayout();
    void CreateSpritePipelineLayout();
    void CreateToonPipelineLayout();
    // void CreateSubsurfaceScatterPipelineLayout();

  public:
    PipelineLayoutManager(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context);
    vk::PipelineLayout GetPipelineLayout(PipelineLayoutType type) const;

  public:
    inline const vk::DescriptorSetLayout &GetGlobalDescriptorSetLayout() const
    {
        return mGlobalDescriptorSetLayout.get();
    }
    inline const vk::DescriptorSetLayout &GetPBRDescriptorSetLayout() const
    {
        return mPBRDescriptorSetLayout.get();
    }
    inline const GlobalLayoutBindings &GetGlobalDescriptorLayoutBindings() const
    {
        return mGlobalDescriptorLayoutBindings;
    }
    inline const PBRLayoutBindings &GetPBRDescriptorLayoutBindings() const
    {
        return mPBRDescriptorLayoutBindings;
    }
};

} // namespace MEngine