#include "PipelineLayoutManager.hpp"
namespace MEngine
{

PipelineLayoutManager::PipelineLayoutManager(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context)
    : mLogger(logger), mContext(context)
{
    CreateMVPDescriptorSetLayout();
    // 创建延迟渲染管线布局
    // CreateDefferPipelineLayout();
    // // 创建阴影深度图管线布局
    // CreateShadowDepthPipelineLayout();
    // 创建半透明物体管线布局
    CreateTranslucencyPipelineLayout();
    // // 创建后处理管线布局
    // CreatePostProcessPipelineLayout();
    // // 创建天空盒管线布局
    // CreateSkyPipelineLayout();
    // // 创建UI管线布局
    // CreateUIPipelineLayout();
}
void PipelineLayoutManager::CreateMVPDescriptorSetLayout()
{
    std::array<vk::DescriptorSetLayoutBinding, 1> MVPDescriptorSetLayoutBindings;
    // Set: 0, Binding: 0 MVP矩阵
    MVPDescriptorSetLayoutBindings[0]
        .setBinding(0)
        .setDescriptorType(vk::DescriptorType::eUniformBuffer)
        .setDescriptorCount(1)
        .setStageFlags(vk::ShaderStageFlagBits::eVertex); // 仅顶点阶段需要
    vk::DescriptorSetLayoutCreateInfo mvpDescriptorSetLayoutCreateInfo{};
    mvpDescriptorSetLayoutCreateInfo.setBindings(MVPDescriptorSetLayoutBindings); // set: 0
    mMVPDescriptorSetLayout = mContext->GetDevice().createDescriptorSetLayoutUnique(mvpDescriptorSetLayoutCreateInfo);
    if (!mMVPDescriptorSetLayout)
    {
        mLogger->Error("Failed to create descriptor set layout for MVP");
    }
}
void PipelineLayoutManager::CreateDefferPipelineLayout()
{
    // 1. 创建描述符集布局
    // PBR
    std::array<vk::DescriptorSetLayoutBinding, 5> PBRDescriptorSetLayoutBindings;
    // Set: 1, Binding 0: Albedo（基础颜色）
    PBRDescriptorSetLayoutBindings[0]
        .setBinding(0)
        .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
        .setDescriptorCount(1)
        .setStageFlags(vk::ShaderStageFlagBits::eFragment);

    // Set: 1, Binding 1: 法线贴图
    PBRDescriptorSetLayoutBindings[1]
        .setBinding(1)
        .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
        .setDescriptorCount(1)
        .setStageFlags(vk::ShaderStageFlagBits::eFragment);

    // Set: 1, Binding 2: 金属度/粗糙度（通常打包在同一个纹理的 RG 通道）
    PBRDescriptorSetLayoutBindings[2]
        .setBinding(2)
        .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
        .setDescriptorCount(1)
        .setStageFlags(vk::ShaderStageFlagBits::eFragment);

    // Set: 1, Binding 3: 环境光遮蔽（AO）
    PBRDescriptorSetLayoutBindings[3]
        .setBinding(3)
        .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
        .setDescriptorCount(1)
        .setStageFlags(vk::ShaderStageFlagBits::eFragment);
    // 2. 创建描述符集布局
    vk::DescriptorSetLayoutCreateInfo pbrDescriptorSetLayoutCreateInfo{};

    pbrDescriptorSetLayoutCreateInfo.setBindings(PBRDescriptorSetLayoutBindings); // set: 1
    auto pbrDescriptorSet = mContext->GetDevice().createDescriptorSetLayoutUnique(pbrDescriptorSetLayoutCreateInfo);
    if (!pbrDescriptorSet)
    {
        mLogger->Error("Failed to create descriptor set layout for DefferPipelineLayout");
    }
    mDescriptorSetLayouts[PipelineLayoutType::DefferLayout] = std::move(pbrDescriptorSet);
    // 3. 创建管线布局
    vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
    std::vector<vk::DescriptorSetLayout> setLayouts{mMVPDescriptorSetLayout.get(),
                                                    mDescriptorSetLayouts[PipelineLayoutType::DefferLayout].get()};
    pipelineLayoutCreateInfo.setSetLayouts(setLayouts);
    auto pipelineLayout = mContext->GetDevice().createPipelineLayoutUnique(pipelineLayoutCreateInfo);
    if (!pipelineLayout)
    {
        mLogger->Error("Failed to create pipeline layout for DefferPipelineLayout");
    }
    mPipelineLayouts[PipelineLayoutType::DefferLayout] = std::move(pipelineLayout);
    mLogger->Debug("Deffer pipeline layout created successfully");
}
void PipelineLayoutManager::CreateShadowDepthPipelineLayout()
{
}
void PipelineLayoutManager::CreateTranslucencyPipelineLayout()
{
    // 创建push constant
    std::array<vk::PushConstantRange, 1> pushConstantRanges;
    pushConstantRanges[0].setOffset(0).setSize(sizeof(glm::mat4x4)).setStageFlags(vk::ShaderStageFlagBits::eVertex);
    // 1. 创建管线布局
    vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.setSetLayouts({mMVPDescriptorSetLayout.get()}).setPushConstantRanges(pushConstantRanges);
    auto pipelineLayout = mContext->GetDevice().createPipelineLayoutUnique(pipelineLayoutCreateInfo);
    if (!pipelineLayout)
    {
        mLogger->Error("Failed to create pipeline layout for TranslucencyPipelineLayout");
    }
    mPipelineLayouts[PipelineLayoutType::TranslucencyLayout] = std::move(pipelineLayout);
    mLogger->Debug("Translucency pipeline layout created successfully");
}
void PipelineLayoutManager::CreatePostProcessPipelineLayout()
{
}
void PipelineLayoutManager::CreateSkyPipelineLayout()
{
}
void PipelineLayoutManager::CreateUIPipelineLayout()
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
vk::DescriptorSetLayout PipelineLayoutManager::GetDescriptorSetLayout(PipelineLayoutType type) const
{
    auto it = mDescriptorSetLayouts.find(type);
    if (it != mDescriptorSetLayouts.end())
    {
        return it->second.get();
    }
    else
    {
        mLogger->Error("Descriptor set layout not found for type {}", static_cast<int>(type));
        return nullptr;
    }
}

} // namespace MEngine