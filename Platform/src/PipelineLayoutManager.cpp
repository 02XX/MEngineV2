#include "PipelineLayoutManager.hpp"

namespace MEngine
{

PipelineLayoutManager::PipelineLayoutManager(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context)
    : mLogger(logger), mContext(context)
{
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

void PipelineLayoutManager::CreateDefferPipelineLayout()
{
    // 1. 创建描述符集布局
    // PBR
    std::array<vk::DescriptorSetLayoutBinding, 5> descriptorSetLayoutBindings;

    // Binding 0: 模型视图投影矩阵（应该用 Uniform Buffer）
    descriptorSetLayoutBindings[0]
        .setBinding(0)
        .setDescriptorType(vk::DescriptorType::eUniformBuffer)
        .setDescriptorCount(1)
        .setStageFlags(vk::ShaderStageFlagBits::eVertex); // 仅顶点阶段需要

    // Binding 1: Albedo（基础颜色）
    descriptorSetLayoutBindings[1]
        .setBinding(1)
        .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
        .setDescriptorCount(1)
        .setStageFlags(vk::ShaderStageFlagBits::eFragment);

    // Binding 2: 法线贴图
    descriptorSetLayoutBindings[2]
        .setBinding(2)
        .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
        .setDescriptorCount(1)
        .setStageFlags(vk::ShaderStageFlagBits::eFragment);

    // Binding 3: 金属度/粗糙度（通常打包在同一个纹理的 RG 通道）
    descriptorSetLayoutBindings[3]
        .setBinding(3)
        .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
        .setDescriptorCount(1)
        .setStageFlags(vk::ShaderStageFlagBits::eFragment);

    // Binding 4: 环境光遮蔽（AO）
    descriptorSetLayoutBindings[4]
        .setBinding(4)
        .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
        .setDescriptorCount(1)
        .setStageFlags(vk::ShaderStageFlagBits::eFragment);
    // 2. 创建描述符集布局
    vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;
    descriptorSetLayoutCreateInfo.setBindings({descriptorSetLayoutBindings}); // set: 0
    auto descriptorSetLayout = mContext->GetDevice().createDescriptorSetLayoutUnique(descriptorSetLayoutCreateInfo);
    if (!descriptorSetLayout)
    {
        mLogger->Error("Failed to create descriptor set layout for DefferPipelineLayout");
    }
    mDescriptorSetLayouts[PipelineLayoutType::DefferLayout] = std::move(descriptorSetLayout);
    // 3. 创建管线布局
    vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
    pipelineLayoutCreateInfo.setSetLayouts(mDescriptorSetLayouts[PipelineLayoutType::DefferLayout].get())
        .setPushConstantRanges(nullptr);
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
    // 1. 创建描述符集布局
    std::array<vk::DescriptorSetLayoutBinding, 1> descriptorSetLayoutBindings;
    // Binding 0: 模型视图投影矩阵（应该用 Uniform Buffer）
    descriptorSetLayoutBindings[0]
        .setBinding(0)
        .setDescriptorType(vk::DescriptorType::eUniformBuffer)
        .setDescriptorCount(1)
        .setStageFlags(vk::ShaderStageFlagBits::eVertex); // 仅顶点阶段需要
    // 2. 创建描述符集布局
    vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;
    descriptorSetLayoutCreateInfo.setBindings({descriptorSetLayoutBindings}); // set: 0
    auto descriptorSetLayout = mContext->GetDevice().createDescriptorSetLayoutUnique(descriptorSetLayoutCreateInfo);
    if (!descriptorSetLayout)
    {
        mLogger->Error("Failed to create descriptor set layout for TranslucencyPipelineLayout");
    }
    mDescriptorSetLayouts[PipelineLayoutType::TranslucencyLayout] = std::move(descriptorSetLayout);
    // 3. 创建管线布局
    vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
    pipelineLayoutCreateInfo.setSetLayouts(mDescriptorSetLayouts[PipelineLayoutType::TranslucencyLayout].get())
        .setPushConstantRanges(nullptr);
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