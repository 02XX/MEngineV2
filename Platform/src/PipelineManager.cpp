#include "PipelineManager.hpp"
#include <memory>

namespace MEngine
{

UniquePipeline PipelineManager::CreateUniqueGraphicsPipeline(const GraphicsPipelineConfig &config)
{
    auto &context = Context::Instance();

    // ========== 1. 顶点输入状态 ==========
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
    vertexInputInfo.setVertexBindingDescriptions(config.vertexBindings)
        .setVertexAttributeDescriptions(config.vertexAttributes);

    // ========== 2. 输入装配状态 ==========
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    inputAssemblyInfo.setTopology(config.topology).setPrimitiveRestartEnable(vk::False);

    // ========== 3. 着色器阶段 ==========
    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = {vk::PipelineShaderStageCreateInfo()
                                                                         .setStage(vk::ShaderStageFlagBits::eVertex)
                                                                         .setModule(config.vertexShader)
                                                                         .setPName("main"),
                                                                     vk::PipelineShaderStageCreateInfo()
                                                                         .setStage(vk::ShaderStageFlagBits::eFragment)
                                                                         .setModule(config.fragmentShader)
                                                                         .setPName("main")};

    // ========== 4. 视口和裁剪 ==========
    vk::PipelineViewportStateCreateInfo viewportInfo;
    viewportInfo.setViewportCount(1).setPViewports(&config.viewport).setScissorCount(1).setPScissors(&config.scissor);

    // ========== 5. 光栅化状态 ==========
    vk::PipelineRasterizationStateCreateInfo rasterizationInfo;
    rasterizationInfo.setDepthClampEnable(vk::False)
        .setRasterizerDiscardEnable(vk::False)
        .setPolygonMode(config.polygonMode)
        .setLineWidth(config.lineWidth)
        .setCullMode(config.cullMode)
        .setFrontFace(config.frontFace)
        .setDepthBiasEnable(vk::False);

    // ========== 6. 多重采样 ==========
    vk::PipelineMultisampleStateCreateInfo multisampleInfo;
    multisampleInfo.setSampleShadingEnable(config.sampleShadingEnable)
        .setRasterizationSamples(config.rasterizationSamples)
        .setMinSampleShading(config.minSampleShading)
        .setPSampleMask(config.sampleMasks.data())
        .setAlphaToCoverageEnable(config.alphaToCoverageEnable)
        .setAlphaToOneEnable(config.alphaToOneEnable);

    // ========== 7. 深度模板测试 ==========
    vk::PipelineDepthStencilStateCreateInfo depthStencilInfo;
    depthStencilInfo.setDepthTestEnable(config.depthTestEnable)
        .setDepthWriteEnable(config.depthWriteEnable)
        .setDepthCompareOp(config.depthCompareOp)
        .setDepthBoundsTestEnable(config.depthBoundsTestEnable)
        .setMinDepthBounds(config.minDepthBounds)
        .setMaxDepthBounds(config.maxDepthBounds)
        .setStencilTestEnable(config.stencilTestEnable)
        .setFront(config.frontStencilOp)
        .setBack(config.backStencilOp);

    // ========== 8. 颜色混合 ==========
    vk::PipelineColorBlendStateCreateInfo colorBlendInfo;
    colorBlendInfo.setLogicOpEnable(config.logicOpEnable)
        .setLogicOp(config.logicOp)
        .setBlendConstants(config.blendConstants);

    // ========== 9. 动态状态 ==========
    std::array dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};

    vk::PipelineDynamicStateCreateInfo dynamicStateInfo;
    dynamicStateInfo.setDynamicStateCount(dynamicStates.size()).setPDynamicStates(dynamicStates.data());

    // ========== 10. 组装管线信息 ==========
    vk::GraphicsPipelineCreateInfo pipelineInfo;
    pipelineInfo.setStageCount(shaderStages.size())
        .setPStages(shaderStages.data())
        .setPVertexInputState(&vertexInputInfo)
        .setPInputAssemblyState(&inputAssemblyInfo)
        .setPViewportState(&viewportInfo)
        .setPRasterizationState(&rasterizationInfo)
        .setPMultisampleState(&multisampleInfo)
        .setPDepthStencilState(&depthStencilInfo)
        .setPColorBlendState(&colorBlendInfo)
        .setPDynamicState(&dynamicStateInfo)
        .setLayout(config.pipelineLayout)
        .setRenderPass(config.renderPass)
        .setSubpass(config.subPass);

    // ========== 11. 创建管线 ==========
    auto result = context.GetDevice()->createGraphicsPipelineUnique(nullptr, // 管线缓存
                                                                pipelineInfo);                                         
    if (result.result != vk::Result::eSuccess)
    {
        LogD("Failed to create graphics pipeline");
        throw std::runtime_error("Failed to create graphics pipeline");
    }
    auto pipeline = std::make_unique<Pipeline>(result.value);
    LogD("Graphics pipeline created");
    return pipeline;
}
SharedPipeline PipelineManager::CreateSharedGraphicsPipeline(const GraphicsPipelineConfig &config)
{
    auto &context = Context::Instance();

    // ========== 1. 顶点输入状态 ==========
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
    vertexInputInfo.setVertexBindingDescriptions(config.vertexBindings)
        .setVertexAttributeDescriptions(config.vertexAttributes);

    // ========== 2. 输入装配状态 ==========
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    inputAssemblyInfo.setTopology(config.topology).setPrimitiveRestartEnable(vk::False);

    // ========== 3. 着色器阶段 ==========
    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = {vk::PipelineShaderStageCreateInfo()
                                                                         .setStage(vk::ShaderStageFlagBits::eVertex)
                                                                         .setModule(config.vertexShader)
                                                                         .setPName("main"),
                                                                     vk::PipelineShaderStageCreateInfo()
                                                                         .setStage(vk::ShaderStageFlagBits::eFragment)
                                                                         .setModule(config.fragmentShader)
                                                                         .setPName("main")};

    // ========== 4. 视口和裁剪 ==========
    vk::PipelineViewportStateCreateInfo viewportInfo;
    viewportInfo.setViewportCount(1).setPViewports(&config.viewport).setScissorCount(1).setPScissors(&config.scissor);

    // ========== 5. 光栅化状态 ==========
    vk::PipelineRasterizationStateCreateInfo rasterizationInfo;
    rasterizationInfo.setDepthClampEnable(vk::False)
        .setRasterizerDiscardEnable(vk::False)
        .setPolygonMode(config.polygonMode)
        .setLineWidth(config.lineWidth)
        .setCullMode(config.cullMode)
        .setFrontFace(config.frontFace)
        .setDepthBiasEnable(vk::False);

    // ========== 6. 多重采样 ==========
    vk::PipelineMultisampleStateCreateInfo multisampleInfo;
    multisampleInfo.setSampleShadingEnable(config.sampleShadingEnable)
        .setRasterizationSamples(config.rasterizationSamples)
        .setMinSampleShading(config.minSampleShading)
        .setPSampleMask(config.sampleMasks.data())
        .setAlphaToCoverageEnable(config.alphaToCoverageEnable)
        .setAlphaToOneEnable(config.alphaToOneEnable);

    // ========== 7. 深度模板测试 ==========
    vk::PipelineDepthStencilStateCreateInfo depthStencilInfo;
    depthStencilInfo.setDepthTestEnable(config.depthTestEnable)
        .setDepthWriteEnable(config.depthWriteEnable)
        .setDepthCompareOp(config.depthCompareOp)
        .setDepthBoundsTestEnable(config.depthBoundsTestEnable)
        .setMinDepthBounds(config.minDepthBounds)
        .setMaxDepthBounds(config.maxDepthBounds)
        .setStencilTestEnable(config.stencilTestEnable)
        .setFront(config.frontStencilOp)
        .setBack(config.backStencilOp);

    // ========== 8. 颜色混合 ==========
    vk::PipelineColorBlendStateCreateInfo colorBlendInfo;
    colorBlendInfo.setLogicOpEnable(config.logicOpEnable)
        .setLogicOp(config.logicOp)
        .setBlendConstants(config.blendConstants);

    // ========== 9. 动态状态 ==========
    std::array dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};

    vk::PipelineDynamicStateCreateInfo dynamicStateInfo;
    dynamicStateInfo.setDynamicStateCount(dynamicStates.size()).setPDynamicStates(dynamicStates.data());

    // ========== 10. 组装管线信息 ==========
    vk::GraphicsPipelineCreateInfo pipelineInfo;
    pipelineInfo.setStageCount(shaderStages.size())
        .setPStages(shaderStages.data())
        .setPVertexInputState(&vertexInputInfo)
        .setPInputAssemblyState(&inputAssemblyInfo)
        .setPViewportState(&viewportInfo)
        .setPRasterizationState(&rasterizationInfo)
        .setPMultisampleState(&multisampleInfo)
        .setPDepthStencilState(&depthStencilInfo)
        .setPColorBlendState(&colorBlendInfo)
        .setPDynamicState(&dynamicStateInfo)
        .setLayout(config.pipelineLayout)
        .setRenderPass(config.renderPass)
        .setSubpass(config.subPass);

    // ========== 11. 创建管线 ==========
    auto result = context.GetDevice()->createGraphicsPipeline(nullptr, // 管线缓存
                                                              pipelineInfo);

    if (result.result != vk::Result::eSuccess)
    {
        LogD("Failed to create graphics pipeline");
        throw std::runtime_error("Failed to create graphics pipeline");
    }
    LogD("Graphics pipeline created");
    auto pipeline = std::make_shared<Pipeline>(result.value);
    return pipeline;
}
UniquePipeline PipelineManager::CreateComputePipeline(const ComputePipelineConfig &config)
{
}
} // namespace MEngine