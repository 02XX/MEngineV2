#include "PipelineLayoutManager.hpp"
namespace MEngine
{

PipelineLayoutManager::PipelineLayoutManager(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context)
    : mLogger(logger), mContext(context)
{
    // DescriptorSetLayout
    CreateGlobalDescriptorSetLayout();
    CreatePBRDescriptorSetLayout();
    CreatePhongDescriptorSetLayout();
    // PipelineLayout
    CreateShadowDepthPipelineLayout();
    CreatePBRPipelineLayout();
    CreatePhongPipelineLayout();
    CreateScreenSpaceEffectPipelineLayout();
    CreateSkyPipelineLayout();
    CreateParticlePipelineLayout();
    CreateTerrainPipelineLayout();
    CreateSkinnedMeshPipelineLayout();
    CreatePostProcessPipelineLayout();
    CreateUIPipelineLayout();
    CreateSpritePipelineLayout();
    CreateToonPipelineLayout();
}
// DescriptorSetLayout
void PipelineLayoutManager::CreateGlobalDescriptorSetLayout()
{
    std::vector<vk::DescriptorSetLayoutBinding> globalDescriptorSetLayoutBindings{
        mGlobalDescriptorLayoutBindings.mCameraBinding, mGlobalDescriptorLayoutBindings.mLightBinding,
        mGlobalDescriptorLayoutBindings.mEnvMapBinding, mGlobalDescriptorLayoutBindings.mShadowParametersBinding,
        mGlobalDescriptorLayoutBindings.mShadowMapsBinding};
    vk::DescriptorSetLayoutCreateInfo globalDescriptorSetLayoutCreateInfo{};
    globalDescriptorSetLayoutCreateInfo.setBindings(globalDescriptorSetLayoutBindings); // set: 0
    mGlobalDescriptorSetLayout =
        mContext->GetDevice().createDescriptorSetLayoutUnique(globalDescriptorSetLayoutCreateInfo);
    if (!mGlobalDescriptorSetLayout)
    {
        mLogger->Error("Failed to create descriptor set layout for MVP");
    }
    mLogger->Info("Global descriptor set layout created successfully");
}
void PipelineLayoutManager::CreatePBRDescriptorSetLayout()
{
    std::vector<vk::DescriptorSetLayoutBinding> pbrDescriptorSetLayoutBindings{
        mPBRDescriptorLayoutBindings.mParameterBinding,        mPBRDescriptorLayoutBindings.mBaseColorBinding,
        mPBRDescriptorLayoutBindings.mNormalMapBinding,        mPBRDescriptorLayoutBindings.mMetallicRoughnessBinding,
        mPBRDescriptorLayoutBindings.mAmbientOcclusionBinding, mPBRDescriptorLayoutBindings.mEmissiveBinding};
    vk::DescriptorSetLayoutCreateInfo pbrDescriptorSetLayoutCreateInfo{};
    pbrDescriptorSetLayoutCreateInfo.setBindings(pbrDescriptorSetLayoutBindings)
        .setFlags(vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool); // set: 1
    mPBRDescriptorSetLayout = mContext->GetDevice().createDescriptorSetLayoutUnique(pbrDescriptorSetLayoutCreateInfo);
    if (!mPBRDescriptorSetLayout)
    {
        mLogger->Error("Failed to create descriptor set layout for MVP");
    }
    mLogger->Info("PBR descriptor set layout created successfully");
}
void PipelineLayoutManager::CreatePhongDescriptorSetLayout()
{
}
// PipelineLayout
void PipelineLayoutManager::CreateShadowDepthPipelineLayout()
{
}
void PipelineLayoutManager::CreatePhongPipelineLayout()
{
}
void PipelineLayoutManager::CreatePBRPipelineLayout()
{
    vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
    std::vector<vk::DescriptorSetLayout> setLayouts{
        mGlobalDescriptorSetLayout.get(), // set: 0
        mPBRDescriptorSetLayout.get()     // set: 1
    };
    std::array<vk::PushConstantRange, 1> pushConstantRanges;
    pushConstantRanges[0].setOffset(0).setSize(sizeof(glm::mat4x4)).setStageFlags(vk::ShaderStageFlagBits::eVertex);
    pipelineLayoutCreateInfo.setSetLayouts(setLayouts).setPushConstantRanges(pushConstantRanges);
    auto pipelineLayout = mContext->GetDevice().createPipelineLayoutUnique(pipelineLayoutCreateInfo);
    if (!pipelineLayout)
    {
        mLogger->Error("Failed to create pipeline layout for DefferPipelineLayout");
    }
    mPipelineLayouts[PipelineLayoutType::PBR] = std::move(pipelineLayout);
    mLogger->Info("Transparent pipeline layout created successfully");
}
void PipelineLayoutManager::CreateScreenSpaceEffectPipelineLayout()
{
}
void PipelineLayoutManager::CreateSkyPipelineLayout()
{
    vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
    std::vector<vk::DescriptorSetLayout> setLayouts{
        mGlobalDescriptorSetLayout.get(), // set: 0
    };
    pipelineLayoutCreateInfo.setSetLayouts(setLayouts);
    auto pipelineLayout = mContext->GetDevice().createPipelineLayoutUnique(pipelineLayoutCreateInfo);
    if (!pipelineLayout)
    {
        mLogger->Error("Failed to create pipeline layout for DefferPipelineLayout");
    }
    mPipelineLayouts[PipelineLayoutType::Skybox] = std::move(pipelineLayout);
    mLogger->Info("Sky pipeline layout created successfully");
}
void PipelineLayoutManager::CreateParticlePipelineLayout()
{
}
void PipelineLayoutManager::CreateTerrainPipelineLayout()
{
}
void PipelineLayoutManager::CreateSkinnedMeshPipelineLayout()
{
}
void PipelineLayoutManager::CreatePostProcessPipelineLayout()
{
}
void PipelineLayoutManager::CreateUIPipelineLayout()
{
}
void PipelineLayoutManager::CreateSpritePipelineLayout()
{
}
void PipelineLayoutManager::CreateToonPipelineLayout()
{
}
vk::PipelineLayout PipelineLayoutManager::GetPipelineLayout(PipelineLayoutType type) const
{
    auto it = mPipelineLayouts.find(type);
    if (it != mPipelineLayouts.end())
    {
        return it->second.get();
    }
    else
    {
        mLogger->Error("Pipeline layout not found for type {}", static_cast<int>(type));
        return nullptr;
    }
}
} // namespace MEngine