#include "PipelineManager.hpp"
#include <vulkan/vulkan.hpp>

namespace MEngine
{
PipelineManager::PipelineManager(std::shared_ptr<ILogger> logger, std::shared_ptr<Context> context,
                                 std::shared_ptr<ShaderManager> shaderManager,
                                 std::shared_ptr<PipelineLayoutManager> pipelineLayoutManager,
                                 std::shared_ptr<RenderPassManager> renderPassManager)
    : mLogger(logger), mContext(context), mShaderManager(shaderManager), mPipelineLayoutManager(pipelineLayoutManager),
      mRenderPassManager(renderPassManager)
{
    CreateShadowMapPipeline();
    CreateForwardOpaquePBRPipeline();
    CreateForwardOpaquePhongPipeline();
    CreateForwardTransparentPBRPipeline();
    CreateForwardTransparentPhongPipeline();
    CreateDeferredGBufferPipeline();
    CreateDeferredLightingPipeline();
    CreateScreenSpaceEffectSSAOPipeline();
    CreateScreenSpaceEffectSSRPipeline();
    CreateSkinnedMeshPipeline();
    CreateMorphTargetPipeline();
    CreateParticleCPUPipeline();
    CreateParticleGPUPipeline();
    CreateDecalPipeline();
    CreatePostProcessToneMappingPipeline();
    CreatePostProcessBloomPipeline();
    CreatePostProcessDOFPipeline();
    CreatePostProcessMotionBlurPipeline();
    CreatePostProcessFXAAPipeline();
    CreatePostProcessSMAAPipeline();
    CreatePostProcessVignettePipeline();
    CreatePostProcessChromaticAberrationPipeline();
    CreatePostProcessFilmGrainPipeline();
    CreatePostProcessColorGradingPipeline();
    CreateUISpritePipeline();
    CreateUITextPipeline();
    CreateToonPipeline();
    CreateWireframePipeline();
}
void PipelineManager::CreateShadowMapPipeline()
{
}
void PipelineManager::CreateForwardOpaquePBRPipeline()
{
}
void PipelineManager::CreateForwardOpaquePhongPipeline()
{
}
void PipelineManager::CreateForwardTransparentPBRPipeline()
{
    // ========== 1. 顶点输入状态 ==========
    auto vertexBindingDescription = Vertex::GetVertexInputBindingDescription();
    auto vertexInputAttributeDescriptions = Vertex::GetVertexInputAttributeDescription();
    auto vertexAttributeDescriptions = std::vector<vk::VertexInputAttributeDescription>(
        vertexInputAttributeDescriptions.begin(), vertexInputAttributeDescriptions.end());
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.setVertexBindingDescriptions(vertexBindingDescription)
        .setVertexAttributeDescriptions(vertexAttributeDescriptions);
    // ========== 2. 输入装配状态 ==========
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
    inputAssemblyInfo.setTopology(vk::PrimitiveTopology::eTriangleList).setPrimitiveRestartEnable(vk::False);
    // ========== 3. 着色器阶段 ==========
    mShaderManager->LoadShaderModule("TranslucencyVertexShader", "translucency.vert.spv");
    mShaderManager->LoadShaderModule("TranslucencyFragmentShader", "translucency.frag.spv");
    auto vertexShader = mShaderManager->GetShaderModule("TranslucencyVertexShader");
    auto fragmentShader = mShaderManager->GetShaderModule("TranslucencyFragmentShader");
    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = {vk::PipelineShaderStageCreateInfo()
                                                                         .setStage(vk::ShaderStageFlagBits::eVertex)
                                                                         .setModule(vertexShader)
                                                                         .setPName("main"),
                                                                     vk::PipelineShaderStageCreateInfo()
                                                                         .setStage(vk::ShaderStageFlagBits::eFragment)
                                                                         .setModule(fragmentShader)
                                                                         .setPName("main")};
    // ========== 4. 视口和裁剪 ==========
    // Swapchain的宽高和Surface的宽高一致
    vk::Viewport viewport{};
    vk::Rect2D scissor{};
    viewport = vk::Viewport()
                   .setX(0.0f)
                   .setY(0.0f)
                   .setWidth(static_cast<float>(mContext->GetSurfaceInfo().extent.width))
                   .setHeight(static_cast<float>(mContext->GetSurfaceInfo().extent.height))
                   .setMinDepth(0.0f)
                   .setMaxDepth(1.0f);
    scissor = vk::Rect2D().setOffset({0, 0}).setExtent(mContext->GetSurfaceInfo().extent);
    vk::PipelineViewportStateCreateInfo viewportInfo;
    viewportInfo.setViewports(viewport).setScissors(scissor);
    // ========== 5. 光栅化状态 ==========
    vk::PipelineRasterizationStateCreateInfo rasterizationInfo{};
    rasterizationInfo.setDepthClampEnable(vk::False)
        .setRasterizerDiscardEnable(vk::False)
        .setPolygonMode(vk::PolygonMode::eFill)
        .setLineWidth(1.0f)
        .setCullMode(vk::CullModeFlagBits::eBack)
        .setFrontFace(vk::FrontFace::eClockwise)
        .setDepthBiasEnable(vk::False);

    // ========== 6. 多重采样 ==========
    vk::PipelineMultisampleStateCreateInfo multisampleInfo{};
    multisampleInfo.setSampleShadingEnable(vk::False)
        .setRasterizationSamples(vk::SampleCountFlagBits::e1)
        .setMinSampleShading(1.0f)
        .setPSampleMask(nullptr)
        .setAlphaToCoverageEnable(vk::False)
        .setAlphaToOneEnable(vk::False);

    // ========== 7. 深度模板测试 ==========
    vk::PipelineDepthStencilStateCreateInfo depthStencilInfo{};
    vk::StencilOpState frontState;
    frontState
        .setFailOp(vk::StencilOp::eKeep)      // 模板失败：保留原值
        .setPassOp(vk::StencilOp::eReplace)   // 模板和深度通过：替换为参考值
        .setDepthFailOp(vk::StencilOp::eKeep) // 模板通过但深度失败：保留原值
        .setCompareOp(vk::CompareOp::eEqual)  // 比较函数：相等时通过
        .setCompareMask(0xFF)                 // 比较掩码（全位参与比较）
        .setWriteMask(0xFF)                   // 写入掩码（全位允许写入）
        .setReference(1);                     // 参考值（运行时动态设置）
    vk::StencilOpState backState;
    backState
        .setFailOp(vk::StencilOp::eKeep)      // 模板失败：保留原值
        .setPassOp(vk::StencilOp::eReplace)   // 模板和深度通过：替换为参考值
        .setDepthFailOp(vk::StencilOp::eKeep) // 模板通过但深度失败：保留原值
        .setCompareOp(vk::CompareOp::eEqual)  // 比较函数：相等时通过
        .setCompareMask(0xFF)                 // 比较掩码（全位参与比较）
        .setWriteMask(0xFF)                   // 写入掩码（全位允许写入）
        .setReference(1);                     // 参考值（运行时动态设置）
    depthStencilInfo.setDepthTestEnable(vk::True)
        .setDepthWriteEnable(vk::True)
        .setDepthCompareOp(vk::CompareOp::eLessOrEqual)
        .setDepthBoundsTestEnable(vk::False)
        .setMinDepthBounds(0.0f)
        .setMaxDepthBounds(1.0f)
        .setStencilTestEnable(vk::False)
        .setFront(frontState)
        .setBack(backState);

    // ========== 8. 颜色混合 ==========
    std::array<vk::PipelineColorBlendAttachmentState, 1> colorBlendAttachments{
        vk::PipelineColorBlendAttachmentState()
            .setBlendEnable(vk::False) // 是否启用混合
            .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                               vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA) // 写入掩码
            .setSrcColorBlendFactor(vk::BlendFactor::eOne)                                      // 源颜色混合因子
            .setDstColorBlendFactor(vk::BlendFactor::eZero) // 目标颜色混合因子
            .setColorBlendOp(vk::BlendOp::eAdd)             // 混合操作
            .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)  // 源Alpha混合因子
            .setDstAlphaBlendFactor(vk::BlendFactor::eZero) // 目标Alpha混合因子
            .setAlphaBlendOp(vk::BlendOp::eAdd)};
    auto blendConstants = std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f}; // 混合常量
    vk::PipelineColorBlendStateCreateInfo colorBlendInfo{};
    colorBlendInfo.setLogicOpEnable(vk::False)
        .setAttachments(colorBlendAttachments)
        .setLogicOp(vk::LogicOp::eCopy)
        .setBlendConstants(blendConstants);
    // ========== 9. 动态状态 ==========
    std::array dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
    vk::PipelineDynamicStateCreateInfo dynamicStateInfo{};
    dynamicStateInfo.setDynamicStates(dynamicStates);
    // ========== 10. 管线创建 ==========
    vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
    graphicsPipelineCreateInfo.setPViewportState(&viewportInfo)
        .setPInputAssemblyState(&inputAssemblyInfo)
        .setPVertexInputState(&vertexInputInfo)
        .setStages(shaderStages)
        .setLayout(mPipelineLayoutManager->GetPipelineLayout(PipelineLayoutType::PBR))
        .setRenderPass(mRenderPassManager->GetRenderPass(RenderPassType::Transparent))
        .setSubpass(0)
        .setPRasterizationState(&rasterizationInfo)
        .setPMultisampleState(&multisampleInfo)
        .setPDepthStencilState(&depthStencilInfo)
        .setPColorBlendState(&colorBlendInfo)
        .setPDynamicState(&dynamicStateInfo);
    auto pipeline = mContext->GetDevice().createGraphicsPipelineUnique(nullptr, graphicsPipelineCreateInfo);
    if (pipeline.result != vk::Result::eSuccess)
    {
        mLogger->Error("Failed to create ForwardTransparent pipeline");
    }
    mPipelines[PipelineType::ForwardTransparentPBR] = std::move(pipeline.value);
    mLogger->Info("Create ForwardTransparent pipeline success");
}
void PipelineManager::CreateForwardTransparentPhongPipeline()
{
}
void PipelineManager::CreateDeferredGBufferPipeline()
{
    // ========== 1. 顶点输入状态 ==========
    auto vertexBindingDescription = Vertex::GetVertexInputBindingDescription();
    auto vertexInputAttributeDescriptions = Vertex::GetVertexInputAttributeDescription();
    auto vertexAttributeDescriptions = std::vector<vk::VertexInputAttributeDescription>(
        vertexInputAttributeDescriptions.begin(), vertexInputAttributeDescriptions.end());
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.setVertexBindingDescriptions(vertexBindingDescription)
        .setVertexAttributeDescriptions(vertexAttributeDescriptions);
    // ========== 2. 输入装配状态 ==========
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
    inputAssemblyInfo.setTopology(vk::PrimitiveTopology::eTriangleList).setPrimitiveRestartEnable(vk::False);
    // ========== 3. 着色器阶段 ==========
    mShaderManager->LoadShaderModule("GBufferVertexShader", "gBuffer.vert.spv");
    mShaderManager->LoadShaderModule("GBufferFragmentShader", "gBuffer.frag.spv");
    auto vertexShader = mShaderManager->GetShaderModule("GBufferVertexShader");
    auto fragmentShader = mShaderManager->GetShaderModule("GBufferFragmentShader");
    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = {vk::PipelineShaderStageCreateInfo()
                                                                         .setStage(vk::ShaderStageFlagBits::eVertex)
                                                                         .setModule(vertexShader)
                                                                         .setPName("main"),
                                                                     vk::PipelineShaderStageCreateInfo()
                                                                         .setStage(vk::ShaderStageFlagBits::eFragment)
                                                                         .setModule(fragmentShader)
                                                                         .setPName("main")};
    // ========== 4. 视口和裁剪 ==========
    // Swapchain的宽高和Surface的宽高一致
    vk::Viewport viewport{};
    vk::Rect2D scissor{};
    viewport = vk::Viewport()
                   .setX(0.0f)
                   .setY(0.0f)
                   .setWidth(static_cast<float>(mContext->GetSurfaceInfo().extent.width))
                   .setHeight(static_cast<float>(mContext->GetSurfaceInfo().extent.height))
                   .setMinDepth(0.0f)
                   .setMaxDepth(1.0f);
    scissor = vk::Rect2D().setOffset({0, 0}).setExtent(mContext->GetSurfaceInfo().extent);
    vk::PipelineViewportStateCreateInfo viewportInfo;
    viewportInfo.setViewports(viewport).setScissors(scissor);
    // ========== 5. 光栅化状态 ==========
    vk::PipelineRasterizationStateCreateInfo rasterizationInfo{};
    rasterizationInfo.setDepthClampEnable(vk::False)
        .setRasterizerDiscardEnable(vk::False)
        .setPolygonMode(vk::PolygonMode::eFill)
        .setLineWidth(1.0f)
        .setCullMode(vk::CullModeFlagBits::eBack)
        .setFrontFace(vk::FrontFace::eClockwise)
        .setDepthBiasEnable(vk::False);

    // ========== 6. 多重采样 ==========
    vk::PipelineMultisampleStateCreateInfo multisampleInfo{};
    multisampleInfo.setSampleShadingEnable(vk::False)
        .setRasterizationSamples(vk::SampleCountFlagBits::e1)
        .setMinSampleShading(1.0f)
        .setPSampleMask(nullptr)
        .setAlphaToCoverageEnable(vk::False)
        .setAlphaToOneEnable(vk::False);

    // ========== 7. 深度模板测试 ==========
    vk::PipelineDepthStencilStateCreateInfo depthStencilInfo{};
    vk::StencilOpState frontState;
    frontState
        .setFailOp(vk::StencilOp::eKeep)      // 模板失败：保留原值
        .setPassOp(vk::StencilOp::eReplace)   // 模板和深度通过：替换为参考值
        .setDepthFailOp(vk::StencilOp::eKeep) // 模板通过但深度失败：保留原值
        .setCompareOp(vk::CompareOp::eEqual)  // 比较函数：相等时通过
        .setCompareMask(0xFF)                 // 比较掩码（全位参与比较）
        .setWriteMask(0xFF)                   // 写入掩码（全位允许写入）
        .setReference(1);                     // 参考值（运行时动态设置）
    vk::StencilOpState backState;
    backState
        .setFailOp(vk::StencilOp::eKeep)      // 模板失败：保留原值
        .setPassOp(vk::StencilOp::eReplace)   // 模板和深度通过：替换为参考值
        .setDepthFailOp(vk::StencilOp::eKeep) // 模板通过但深度失败：保留原值
        .setCompareOp(vk::CompareOp::eEqual)  // 比较函数：相等时通过
        .setCompareMask(0xFF)                 // 比较掩码（全位参与比较）
        .setWriteMask(0xFF)                   // 写入掩码（全位允许写入）
        .setReference(1);                     // 参考值（运行时动态设置）
    depthStencilInfo.setDepthTestEnable(vk::True)
        .setDepthWriteEnable(vk::True)
        .setDepthCompareOp(vk::CompareOp::eLessOrEqual)
        .setDepthBoundsTestEnable(vk::False)
        .setMinDepthBounds(0.0f)
        .setMaxDepthBounds(1.0f)
        .setStencilTestEnable(vk::False)
        .setFront(frontState)
        .setBack(backState);

    // ========== 8. 颜色混合 ==========
    // renderTarget
    vk::PipelineColorBlendAttachmentState renderTargetBlendAttachmentState{};
    renderTargetBlendAttachmentState
        .setBlendEnable(vk::False) // 是否启用混合
        .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                           vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA) // 写入掩码
        .setSrcColorBlendFactor(vk::BlendFactor::eOne)                                      // 源颜色混合因子
        .setDstColorBlendFactor(vk::BlendFactor::eZero)                                     // 目标颜色混合因子
        .setColorBlendOp(vk::BlendOp::eAdd)                                                 // 混合操作
        .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)                                      // 源Alpha混合因子
        .setDstAlphaBlendFactor(vk::BlendFactor::eZero)                                     // 目标Alpha混合因子
        .setAlphaBlendOp(vk::BlendOp::eAdd);
    // albedo
    vk::PipelineColorBlendAttachmentState albedoBlendAttachmentState{};
    albedoBlendAttachmentState
        .setBlendEnable(vk::False) // 是否启用混合
        .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                           vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA) // 写入掩码
        .setSrcColorBlendFactor(vk::BlendFactor::eOne)                                      // 源颜色混合因子
        .setDstColorBlendFactor(vk::BlendFactor::eZero)                                     // 目标颜色混合因子
        .setColorBlendOp(vk::BlendOp::eAdd)                                                 // 混合操作
        .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)                                      // 源Alpha混合因子
        .setDstAlphaBlendFactor(vk::BlendFactor::eZero)                                     // 目标Alpha混合因子
        .setAlphaBlendOp(vk::BlendOp::eAdd);
    // position
    vk::PipelineColorBlendAttachmentState positionBlendAttachmentState{};
    positionBlendAttachmentState
        .setBlendEnable(vk::False) // 是否启用混合
        .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                           vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA) // 写入掩码
        .setSrcColorBlendFactor(vk::BlendFactor::eOne)                                      // 源颜色混合因子
        .setDstColorBlendFactor(vk::BlendFactor::eZero)                                     // 目标颜色混合因子
        .setColorBlendOp(vk::BlendOp::eAdd)                                                 // 混合操作
        .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)                                      // 源Alpha混合因子
        .setDstAlphaBlendFactor(vk::BlendFactor::eZero)                                     // 目标Alpha混合因子
        .setAlphaBlendOp(vk::BlendOp::eAdd);
    // normal
    vk::PipelineColorBlendAttachmentState normalBlendAttachmentState{};
    normalBlendAttachmentState
        .setBlendEnable(vk::False) // 是否启用混合
        .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                           vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA) // 写入掩码
        .setSrcColorBlendFactor(vk::BlendFactor::eOne)                                      // 源颜色混合因子
        .setDstColorBlendFactor(vk::BlendFactor::eZero)                                     // 目标颜色混合因子
        .setColorBlendOp(vk::BlendOp::eAdd)                                                 // 混合操作
        .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)                                      // 源Alpha混合因子
        .setDstAlphaBlendFactor(vk::BlendFactor::eZero)                                     // 目标Alpha混合因子
        .setAlphaBlendOp(vk::BlendOp::eAdd);
    // Metalness/Roughness
    vk::PipelineColorBlendAttachmentState metalnessRoughnessBlendAttachmentState{};
    metalnessRoughnessBlendAttachmentState
        .setBlendEnable(vk::False) // 是否启用混合
        .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                           vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA) // 写入掩码
        .setSrcColorBlendFactor(vk::BlendFactor::eOne)                                      // 源颜色混合因子
        .setDstColorBlendFactor(vk::BlendFactor::eZero)                                     // 目标颜色混合因子
        .setColorBlendOp(vk::BlendOp::eAdd)                                                 // 混合操作
        .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)                                      // 源Alpha混合因子
        .setDstAlphaBlendFactor(vk::BlendFactor::eZero)                                     // 目标Alpha混合因子
        .setAlphaBlendOp(vk::BlendOp::eAdd);
    // AO
    vk::PipelineColorBlendAttachmentState aoBlendAttachmentState{};
    aoBlendAttachmentState
        .setBlendEnable(vk::False) // 是否启用混合
        .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                           vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA) // 写入掩码
        .setSrcColorBlendFactor(vk::BlendFactor::eOne)                                      // 源颜色混合因子
        .setDstColorBlendFactor(vk::BlendFactor::eZero)                                     // 目标颜色混合因子
        .setColorBlendOp(vk::BlendOp::eAdd)                                                 // 混合操作
        .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)                                      // 源Alpha混合因子
        .setDstAlphaBlendFactor(vk::BlendFactor::eZero)                                     // 目标Alpha混合因子
        .setAlphaBlendOp(vk::BlendOp::eAdd);
    // emissive
    vk::PipelineColorBlendAttachmentState emissiveBlendAttachmentState{};
    emissiveBlendAttachmentState
        .setBlendEnable(vk::False) // 是否启用混合
        .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                           vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA) // 写入掩码
        .setSrcColorBlendFactor(vk::BlendFactor::eOne)                                      // 源颜色混合因子
        .setDstColorBlendFactor(vk::BlendFactor::eZero)                                     // 目标颜色混合因子
        .setColorBlendOp(vk::BlendOp::eAdd)                                                 // 混合操作
        .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)                                      // 源Alpha混合因子
        .setDstAlphaBlendFactor(vk::BlendFactor::eZero)                                     // 目标Alpha混合因子
        .setAlphaBlendOp(vk::BlendOp::eAdd);
    std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachments{
        albedoBlendAttachmentState, positionBlendAttachmentState,
        normalBlendAttachmentState, metalnessRoughnessBlendAttachmentState,
        aoBlendAttachmentState,     emissiveBlendAttachmentState};
    auto blendConstants = std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f}; // 混合常量
    vk::PipelineColorBlendStateCreateInfo colorBlendInfo{};
    colorBlendInfo.setLogicOpEnable(vk::False)
        .setAttachments(colorBlendAttachments)
        .setLogicOp(vk::LogicOp::eCopy)
        .setBlendConstants(blendConstants);
    // ========== 9. 动态状态 ==========
    std::array dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
    vk::PipelineDynamicStateCreateInfo dynamicStateInfo{};
    dynamicStateInfo.setDynamicStates(dynamicStates);
    // ========== 10. 管线创建 ==========
    vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
    graphicsPipelineCreateInfo.setPViewportState(&viewportInfo)
        .setPInputAssemblyState(&inputAssemblyInfo)
        .setPVertexInputState(&vertexInputInfo)
        .setStages(shaderStages)
        .setLayout(mPipelineLayoutManager->GetPipelineLayout(PipelineLayoutType::PBR))
        .setRenderPass(mRenderPassManager->GetRenderPass(RenderPassType::DeferredComposition))
        .setSubpass(0)
        .setPRasterizationState(&rasterizationInfo)
        .setPMultisampleState(&multisampleInfo)
        .setPDepthStencilState(&depthStencilInfo)
        .setPColorBlendState(&colorBlendInfo)
        .setPDynamicState(&dynamicStateInfo);
    auto pipeline = mContext->GetDevice().createGraphicsPipelineUnique(nullptr, graphicsPipelineCreateInfo);
    if (pipeline.result != vk::Result::eSuccess)
    {
        mLogger->Error("Failed to create DeferredGBuffer pipeline");
    }
    mPipelines[PipelineType::ForwardTransparentPBR] = std::move(pipeline.value);
    mLogger->Info("Create DeferredGBuffer pipeline success");
}
void PipelineManager::CreateDeferredLightingPipeline()
{
    // ========== 1. 顶点输入状态 ==========
    auto vertexBindingDescription = Vertex::GetVertexInputBindingDescription();
    auto vertexInputAttributeDescriptions = Vertex::GetVertexInputAttributeDescription();
    auto vertexAttributeDescriptions = std::vector<vk::VertexInputAttributeDescription>(
        vertexInputAttributeDescriptions.begin(), vertexInputAttributeDescriptions.end());
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.setVertexBindingDescriptions(vertexBindingDescription)
        .setVertexAttributeDescriptions(vertexAttributeDescriptions);
    // ========== 2. 输入装配状态 ==========
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
    inputAssemblyInfo.setTopology(vk::PrimitiveTopology::eTriangleList).setPrimitiveRestartEnable(vk::False);
    // ========== 3. 着色器阶段 ==========
    mShaderManager->LoadShaderModule("LightingVertexShader", "lighting.vert.spv");
    mShaderManager->LoadShaderModule("LightingFragmentShader", "lighting.frag.spv");
    auto vertexShader = mShaderManager->GetShaderModule("LightingVertexShader");
    auto fragmentShader = mShaderManager->GetShaderModule("LightingFragmentShader");
    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = {vk::PipelineShaderStageCreateInfo()
                                                                         .setStage(vk::ShaderStageFlagBits::eVertex)
                                                                         .setModule(vertexShader)
                                                                         .setPName("main"),
                                                                     vk::PipelineShaderStageCreateInfo()
                                                                         .setStage(vk::ShaderStageFlagBits::eFragment)
                                                                         .setModule(fragmentShader)
                                                                         .setPName("main")};
    // ========== 4. 视口和裁剪 ==========
    // Swapchain的宽高和Surface的宽高一致
    vk::Viewport viewport{};
    vk::Rect2D scissor{};
    viewport = vk::Viewport()
                   .setX(0.0f)
                   .setY(0.0f)
                   .setWidth(static_cast<float>(mContext->GetSurfaceInfo().extent.width))
                   .setHeight(static_cast<float>(mContext->GetSurfaceInfo().extent.height))
                   .setMinDepth(0.0f)
                   .setMaxDepth(1.0f);
    scissor = vk::Rect2D().setOffset({0, 0}).setExtent(mContext->GetSurfaceInfo().extent);
    vk::PipelineViewportStateCreateInfo viewportInfo;
    viewportInfo.setViewports(viewport).setScissors(scissor);
    // ========== 5. 光栅化状态 ==========
    vk::PipelineRasterizationStateCreateInfo rasterizationInfo{};
    rasterizationInfo.setDepthClampEnable(vk::False)
        .setRasterizerDiscardEnable(vk::False)
        .setPolygonMode(vk::PolygonMode::eFill)
        .setLineWidth(1.0f)
        .setCullMode(vk::CullModeFlagBits::eBack)
        .setFrontFace(vk::FrontFace::eClockwise)
        .setDepthBiasEnable(vk::False);

    // ========== 6. 多重采样 ==========
    vk::PipelineMultisampleStateCreateInfo multisampleInfo{};
    multisampleInfo.setSampleShadingEnable(vk::False)
        .setRasterizationSamples(vk::SampleCountFlagBits::e1)
        .setMinSampleShading(1.0f)
        .setPSampleMask(nullptr)
        .setAlphaToCoverageEnable(vk::False)
        .setAlphaToOneEnable(vk::False);

    // ========== 7. 深度模板测试 ==========
    vk::PipelineDepthStencilStateCreateInfo depthStencilInfo{};
    vk::StencilOpState frontState;
    frontState
        .setFailOp(vk::StencilOp::eKeep)      // 模板失败：保留原值
        .setPassOp(vk::StencilOp::eReplace)   // 模板和深度通过：替换为参考值
        .setDepthFailOp(vk::StencilOp::eKeep) // 模板通过但深度失败：保留原值
        .setCompareOp(vk::CompareOp::eEqual)  // 比较函数：相等时通过
        .setCompareMask(0xFF)                 // 比较掩码（全位参与比较）
        .setWriteMask(0xFF)                   // 写入掩码（全位允许写入）
        .setReference(1);                     // 参考值（运行时动态设置）
    vk::StencilOpState backState;
    backState
        .setFailOp(vk::StencilOp::eKeep)      // 模板失败：保留原值
        .setPassOp(vk::StencilOp::eReplace)   // 模板和深度通过：替换为参考值
        .setDepthFailOp(vk::StencilOp::eKeep) // 模板通过但深度失败：保留原值
        .setCompareOp(vk::CompareOp::eEqual)  // 比较函数：相等时通过
        .setCompareMask(0xFF)                 // 比较掩码（全位参与比较）
        .setWriteMask(0xFF)                   // 写入掩码（全位允许写入）
        .setReference(1);                     // 参考值（运行时动态设置）
    depthStencilInfo.setDepthTestEnable(vk::False)
        .setDepthWriteEnable(vk::False)
        .setDepthCompareOp(vk::CompareOp::eLessOrEqual)
        .setDepthBoundsTestEnable(vk::False)
        .setMinDepthBounds(0.0f)
        .setMaxDepthBounds(1.0f)
        .setStencilTestEnable(vk::False)
        .setFront(frontState)
        .setBack(backState);

    // ========== 8. 颜色混合 ==========
    // renderTarget
    vk::PipelineColorBlendAttachmentState renderTargetBlendAttachmentState{};
    renderTargetBlendAttachmentState
        .setBlendEnable(vk::False) // 是否启用混合
        .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                           vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA) // 写入掩码
        .setSrcColorBlendFactor(vk::BlendFactor::eOne)                                      // 源颜色混合因子
        .setDstColorBlendFactor(vk::BlendFactor::eZero)                                     // 目标颜色混合因子
        .setColorBlendOp(vk::BlendOp::eAdd)                                                 // 混合操作
        .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)                                      // 源Alpha混合因子
        .setDstAlphaBlendFactor(vk::BlendFactor::eZero)                                     // 目标Alpha混合因子
        .setAlphaBlendOp(vk::BlendOp::eAdd);
    std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachments{renderTargetBlendAttachmentState};

    auto blendConstants = std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f}; // 混合常量
    vk::PipelineColorBlendStateCreateInfo colorBlendInfo{};
    colorBlendInfo.setLogicOpEnable(vk::False)
        .setAttachments(colorBlendAttachments)
        .setLogicOp(vk::LogicOp::eCopy)
        .setBlendConstants(blendConstants);
    // ========== 9. 动态状态 ==========
    std::array dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
    vk::PipelineDynamicStateCreateInfo dynamicStateInfo{};
    dynamicStateInfo.setDynamicStates(dynamicStates);
    // ========== 10. 管线创建 ==========
    vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
    graphicsPipelineCreateInfo.setPViewportState(&viewportInfo)
        .setPInputAssemblyState(&inputAssemblyInfo)
        .setPVertexInputState(&vertexInputInfo)
        .setStages(shaderStages)
        .setLayout(mPipelineLayoutManager->GetPipelineLayout(PipelineLayoutType::DeferredLighting))
        .setRenderPass(mRenderPassManager->GetRenderPass(RenderPassType::DeferredComposition))
        .setSubpass(1)
        .setPRasterizationState(&rasterizationInfo)
        .setPMultisampleState(&multisampleInfo)
        .setPDepthStencilState(&depthStencilInfo)
        .setPColorBlendState(&colorBlendInfo)
        .setPDynamicState(&dynamicStateInfo);
    auto pipeline = mContext->GetDevice().createGraphicsPipelineUnique(nullptr, graphicsPipelineCreateInfo);
    if (pipeline.result != vk::Result::eSuccess)
    {
        mLogger->Error("Failed to create DeferredLighting pipeline");
    }
    mPipelines[PipelineType::ForwardTransparentPBR] = std::move(pipeline.value);
    mLogger->Info("Create DeferredLighting pipeline success");
}
void PipelineManager::CreateScreenSpaceEffectSSAOPipeline()
{
}
void PipelineManager::CreateScreenSpaceEffectSSRPipeline()
{
}
void PipelineManager::CreateSkinnedMeshPipeline()
{
}
void PipelineManager::CreateMorphTargetPipeline()
{
}
void PipelineManager::CreateParticleCPUPipeline()
{
}
void PipelineManager::CreateParticleGPUPipeline()
{
}
void PipelineManager::CreateDecalPipeline()
{
}
void PipelineManager::CreatePostProcessToneMappingPipeline()
{
}
void PipelineManager::CreatePostProcessBloomPipeline()
{
}
void PipelineManager::CreatePostProcessDOFPipeline()
{
}
void PipelineManager::CreatePostProcessMotionBlurPipeline()
{
}
void PipelineManager::CreatePostProcessFXAAPipeline()
{
}
void PipelineManager::CreatePostProcessSMAAPipeline()
{
}
void PipelineManager::CreatePostProcessVignettePipeline()
{
}
void PipelineManager::CreatePostProcessChromaticAberrationPipeline()
{
}
void PipelineManager::CreatePostProcessFilmGrainPipeline()
{
}
void PipelineManager::CreatePostProcessColorGradingPipeline()
{
}
void PipelineManager::CreateUISpritePipeline()
{
}
void PipelineManager::CreateUITextPipeline()
{
}
void PipelineManager::CreateToonPipeline()
{
}
void PipelineManager::CreateWireframePipeline()
{
}

vk::Pipeline PipelineManager::GetPipeline(PipelineType type) const
{
    auto it = mPipelines.find(type);
    if (it != mPipelines.end())
    {
        return it->second.get();
    }
    else
    {
        mLogger->Error("Pipeline not found: {}", static_cast<int>(type));
        return nullptr;
    }
}
} // namespace MEngine