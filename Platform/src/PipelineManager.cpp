#include "PipelineManager.hpp"
#include "Logger.hpp"
#include "PipelineLayoutManager.hpp"
#include <vulkan/vulkan.hpp>

namespace MEngine
{
PipelineManager::PipelineManager(std::shared_ptr<ShaderManager> shaderManager,
                                 std::shared_ptr<PipelineLayoutManager> pipelineLayoutManager,
                                 std::shared_ptr<RenderPassManager> renderPassManager)
    : mShaderManager(shaderManager), mPipelineLayoutManager(pipelineLayoutManager),
      mRenderPassManager(renderPassManager)
{
    // 创建延迟渲染管线
    // CreateGBufferPipeline();
    // // 创建阴影深度图管线
    // CreateShadowDepthPipeline();
    // // 创建光照管线
    // CreateLightingPipeline();
    // // 创建半透明物体管线
    // CreateTranslucencyPipeline();
    // // 创建后处理管线
    // CreatePostProcessPipeline();
    // // 创建天空盒管线
    // CreateSkyPipeline();
    // // 创建UI管线
    // CreateUIPipeline();
}
void PipelineManager::CommonSetting()
{
    auto &context = Context::Instance();
    // ========== 1. 顶点输入状态 ==========
    mVertexBindingDescription = Vertex::GetVertexInputBindingDescription();
    auto vertexInputAttributeDescriptions = Vertex::GetVertexInputAttributeDescription();
    mVertexAttributeDescriptions = std::vector<vk::VertexInputAttributeDescription>(
        vertexInputAttributeDescriptions.begin(), vertexInputAttributeDescriptions.end());
    mVertexInputInfo.setVertexBindingDescriptions(mVertexBindingDescription)
        .setVertexAttributeDescriptions(mVertexAttributeDescriptions);
    // ========== 2. 输入装配状态 ==========
    mInputAssemblyInfo.setTopology(vk::PrimitiveTopology::eTriangleList).setPrimitiveRestartEnable(vk::False);

    // ========== 4. 视口和裁剪 ==========
    // Swapchain的宽高和Surface的宽高一致
    mViewport = vk::Viewport()
                    .setX(0.0f)
                    .setY(0.0f)
                    .setWidth(static_cast<float>(context.GetSurfaceInfo().extent.width))
                    .setHeight(static_cast<float>(context.GetSurfaceInfo().extent.height))
                    .setMinDepth(0.0f)
                    .setMaxDepth(1.0f);
    mScissor = vk::Rect2D().setOffset({0, 0}).setExtent(context.GetSurfaceInfo().extent);
    mViewportInfo.setViewportCount(1).setPViewports(&mViewport).setScissorCount(1).setPScissors(&mScissor);
    // ========== 5. 光栅化状态 ==========
    mRasterizationInfo.setDepthClampEnable(vk::False)
        .setRasterizerDiscardEnable(vk::False)
        .setPolygonMode(vk::PolygonMode::eFill)
        .setLineWidth(1.0f)
        .setCullMode(vk::CullModeFlagBits::eBack)
        .setFrontFace(vk::FrontFace::eCounterClockwise)
        .setDepthBiasEnable(vk::False);

    // ========== 6. 多重采样 ==========
    mMultisampleInfo.setSampleShadingEnable(vk::False)
        .setRasterizationSamples(vk::SampleCountFlagBits::e1)
        .setMinSampleShading(1.0f)
        .setPSampleMask(nullptr)
        .setAlphaToCoverageEnable(vk::False)
        .setAlphaToOneEnable(vk::False);

    // ========== 7. 深度模板测试 ==========
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
    mDepthStencilInfo.setDepthTestEnable(vk::True)
        .setDepthWriteEnable(vk::True)
        .setDepthCompareOp(vk::CompareOp::eLessOrEqual)
        .setDepthBoundsTestEnable(vk::False)
        .setMinDepthBounds(0.0f)
        .setMaxDepthBounds(1.0f)
        .setStencilTestEnable(vk::False)
        .setFront(frontState)
        .setBack(backState);

    // ========== 8. 颜色混合 ==========
    std::array<vk::PipelineColorBlendAttachmentState, 1> colorBlendAttachments;
    colorBlendAttachments[0]
        .setBlendEnable(vk::True) // 是否启用混合
        .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                           vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA) // 写入掩码
        .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)                                 // 源颜色混合因子
        .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)                         // 目标颜色混合因子
        .setColorBlendOp(vk::BlendOp::eAdd)                                                 // 混合操作
        .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)                                      // 源Alpha混合因子
        .setDstAlphaBlendFactor(vk::BlendFactor::eZero)                                     // 目标Alpha混合因子
        .setAlphaBlendOp(vk::BlendOp::eAdd);                                                // Alpha混合操作
    auto blendConstants = std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f};                     // 混合常量
    mColorBlendInfo.setLogicOpEnable(vk::False)
        .setAttachments(colorBlendAttachments)
        .setLogicOp(vk::LogicOp::eCopy)
        .setBlendConstants(blendConstants);
    // ========== 9. 动态状态 ==========
    // std::array dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
    mConfig.setPViewportState(&mViewportInfo)
        .setPInputAssemblyState(&mInputAssemblyInfo)
        .setPVertexInputState(&mVertexInputInfo)
        .setPRasterizationState(&mRasterizationInfo)
        .setPMultisampleState(&mMultisampleInfo)
        .setPDepthStencilState(&mDepthStencilInfo)
        .setPColorBlendState(&mColorBlendInfo);
}
void PipelineManager::CreateGBufferPipeline()
{
    mConfig = vk::GraphicsPipelineCreateInfo{};
    CommonSetting();
    auto &context = Context::Instance();
    // ========== 3. 着色器阶段 ==========
    mShaderManager->LoadShaderModule("GBufferVertexShader", "shaders/gbuffer.vert.spv");
    mShaderManager->LoadShaderModule("GBufferFragmentShader", "shaders/gbuffer.frag.spv");
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
    std::array<vk::PipelineColorBlendAttachmentState, 5> colorBlendAttachments;
    // 1.1 位置附件
    colorBlendAttachments[0].setBlendEnable(vk::False).setColorWriteMask(
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
        vk::ColorComponentFlagBits::eA);
    // 1.2 法线附件
    colorBlendAttachments[1].setBlendEnable(vk::False).setColorWriteMask(
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
        vk::ColorComponentFlagBits::eA);
    // 1.3 Albedo附件
    colorBlendAttachments[2].setBlendEnable(vk::False).setColorWriteMask(
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
        vk::ColorComponentFlagBits::eA);
    // 1.4 金属度/粗糙度附件
    colorBlendAttachments[3].setBlendEnable(vk::False).setColorWriteMask(
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
        vk::ColorComponentFlagBits::eA);
    // 1.5 环境光遮蔽附件
    colorBlendAttachments[4].setBlendEnable(vk::False).setColorWriteMask(
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
        vk::ColorComponentFlagBits::eA);

    auto blendConstants = std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f}; // 混合常量
    mColorBlendInfo.setLogicOpEnable(vk::False)
        .setAttachments(colorBlendAttachments)
        .setLogicOp(vk::LogicOp::eCopy)
        .setBlendConstants(blendConstants);
    mConfig.setStages(shaderStages)
        .setLayout(mPipelineLayoutManager->GetPipelineLayout(PipelineLayoutType::DefferLayout))
        .setRenderPass(mRenderPassManager->GetRenderPass(RenderPassType::Deffer))
        .setSubpass(0);
    auto pipline = context.GetDevice().createGraphicsPipelineUnique(nullptr, mConfig);
    if (pipline.result != vk::Result::eSuccess)
    {
        LogE("Failed to create GBuffer pipeline");
    }
    mPipelines[PipelineType::DefferGBuffer] = std::move(pipline.value);
    LogD("Create GBuffer pipeline success");
}
void PipelineManager::CreateShadowDepthPipeline()
{
    mConfig = vk::GraphicsPipelineCreateInfo{};
    CommonSetting();
    auto &context = Context::Instance();
    // ========== 3. 着色器阶段 ==========
    mShaderManager->LoadShaderModule("ShadowDepthVertexShader", "shaders/shadowdepth.vert.spv");
    mShaderManager->LoadShaderModule("ShadowDepthFragmentShader", "shaders/shadowdepth.frag.spv");
    auto vertexShader = mShaderManager->GetShaderModule("ShadowDepthVertexShader");
    auto fragmentShader = mShaderManager->GetShaderModule("ShadowDepthFragmentShader");
    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = {vk::PipelineShaderStageCreateInfo()
                                                                         .setStage(vk::ShaderStageFlagBits::eVertex)
                                                                         .setModule(vertexShader)
                                                                         .setPName("main"),
                                                                     vk::PipelineShaderStageCreateInfo()
                                                                         .setStage(vk::ShaderStageFlagBits::eFragment)
                                                                         .setModule(fragmentShader)
                                                                         .setPName("main")};

    mConfig.setStages(shaderStages)
        .setLayout(mPipelineLayoutManager->GetPipelineLayout(PipelineLayoutType::ShadowDepthLayout))
        .setRenderPass(mRenderPassManager->GetRenderPass(RenderPassType::ShadowDepth))
        .setSubpass(0);
    auto pipeline = context.GetDevice().createGraphicsPipelineUnique(nullptr, mConfig);
    if (pipeline.result != vk::Result::eSuccess)
    {
        LogE("Failed to create ShadowDepth pipeline");
    }
    mPipelines[PipelineType::ShadowDepth] = std::move(pipeline.value);
    LogD("Create ShadowDepth pipeline success");
}
void PipelineManager::CreateLightingPipeline()
{
    mConfig = vk::GraphicsPipelineCreateInfo{};
    CommonSetting();
    auto &context = Context::Instance();
    // ========== 3. 着色器阶段 ==========
    mShaderManager->LoadShaderModule("LightingVertexShader", "shaders/lighting.vert.spv");
    mShaderManager->LoadShaderModule("LightingFragmentShader", "shaders/lighting.frag.spv");
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
    mConfig.setStages(shaderStages)
        .setLayout(mPipelineLayoutManager->GetPipelineLayout(PipelineLayoutType::DefferLayout))
        .setRenderPass(mRenderPassManager->GetRenderPass(RenderPassType::Deffer))
        .setSubpass(1);
    auto pipeline = context.GetDevice().createGraphicsPipelineUnique(nullptr, mConfig);
    if (pipeline.result != vk::Result::eSuccess)
    {
        LogE("Failed to create Lighting pipeline");
    }
    mPipelines[PipelineType::DefferLighting] = std::move(pipeline.value);
    LogD("Create Lighting pipeline success");
}
void PipelineManager::CreateTranslucencyPipeline()
{
    mConfig = vk::GraphicsPipelineCreateInfo{};
    CommonSetting();
    auto &context = Context::Instance();
    // ========== 3. 着色器阶段 ==========
    mShaderManager->LoadShaderModule("TranslucencyVertexShader", "shaders/translucency.vert.spv");
    mShaderManager->LoadShaderModule("TranslucencyFragmentShader", "shaders/translucency.frag.spv");
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
    mConfig.setStages(shaderStages)
        .setLayout(mPipelineLayoutManager->GetPipelineLayout(PipelineLayoutType::TranslucencyLayout))
        .setRenderPass(mRenderPassManager->GetRenderPass(RenderPassType::Translucency))
        .setSubpass(0);
    auto pipeline = context.GetDevice().createGraphicsPipelineUnique(nullptr, mConfig);
    if (pipeline.result != vk::Result::eSuccess)
    {
        LogE("Failed to create Translucency pipeline");
    }
    mPipelines[PipelineType::Translucency] = std::move(pipeline.value);
    LogD("Create Translucency pipeline success");
}
void PipelineManager::CreatePostProcessPipeline()
{
    mConfig = vk::GraphicsPipelineCreateInfo{};
    CommonSetting();
    auto &context = Context::Instance();
    // ========== 3. 着色器阶段 ==========
    mShaderManager->LoadShaderModule("PostProcessVertexShader", "shaders/postprocess.vert.spv");
    mShaderManager->LoadShaderModule("PostProcessFragmentShader", "shaders/postprocess.frag.spv");
    auto vertexShader = mShaderManager->GetShaderModule("PostProcessVertexShader");
    auto fragmentShader = mShaderManager->GetShaderModule("PostProcessFragmentShader");
    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = {vk::PipelineShaderStageCreateInfo()
                                                                         .setStage(vk::ShaderStageFlagBits::eVertex)
                                                                         .setModule(vertexShader)
                                                                         .setPName("main"),
                                                                     vk::PipelineShaderStageCreateInfo()
                                                                         .setStage(vk::ShaderStageFlagBits::eFragment)
                                                                         .setModule(fragmentShader)
                                                                         .setPName("main")};
    mConfig.setStages(shaderStages)
        .setLayout(mPipelineLayoutManager->GetPipelineLayout(PipelineLayoutType::PostProcessLayout))
        .setRenderPass(mRenderPassManager->GetRenderPass(RenderPassType::PostProcess))
        .setSubpass(0);
    auto pipeline = context.GetDevice().createGraphicsPipelineUnique(nullptr, mConfig);
    if (pipeline.result != vk::Result::eSuccess)
    {
        LogE("Failed to create PostProcess pipeline");
    }
    mPipelines[PipelineType::PostProcess] = std::move(pipeline.value);
    LogD("Create PostProcess pipeline success");
}
void PipelineManager::CreateSkyPipeline()
{
    mConfig = vk::GraphicsPipelineCreateInfo{};
    CommonSetting();
    auto &context = Context::Instance();
    // ========== 3. 着色器阶段 ==========
    mShaderManager->LoadShaderModule("SkyVertexShader", "shaders/sky.vert.spv");
    mShaderManager->LoadShaderModule("SkyFragmentShader", "shaders/sky.frag.spv");
    auto vertexShader = mShaderManager->GetShaderModule("SkyVertexShader");
    auto fragmentShader = mShaderManager->GetShaderModule("SkyFragmentShader");
    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = {vk::PipelineShaderStageCreateInfo()
                                                                         .setStage(vk::ShaderStageFlagBits::eVertex)
                                                                         .setModule(vertexShader)
                                                                         .setPName("main"),
                                                                     vk::PipelineShaderStageCreateInfo()
                                                                         .setStage(vk::ShaderStageFlagBits::eFragment)
                                                                         .setModule(fragmentShader)
                                                                         .setPName("main")};
    mConfig.setStages(shaderStages)
        .setLayout(mPipelineLayoutManager->GetPipelineLayout(PipelineLayoutType::SkyLayout))
        .setRenderPass(mRenderPassManager->GetRenderPass(RenderPassType::Sky))
        .setSubpass(0);
    auto pipeline = context.GetDevice().createGraphicsPipelineUnique(nullptr, mConfig);
    if (pipeline.result != vk::Result::eSuccess)
    {
        LogE("Failed to create Sky pipeline");
    }
    mPipelines[PipelineType::Sky] = std::move(pipeline.value);
    LogD("Create Sky pipeline success");
}
void PipelineManager::CreateUIPipeline()
{
    mConfig = vk::GraphicsPipelineCreateInfo{};
    CommonSetting();
    auto &context = Context::Instance();
    // ========== 3. 着色器阶段 ==========
    mShaderManager->LoadShaderModule("UIVertexShader", "shaders/ui.vert.spv");
    mShaderManager->LoadShaderModule("UIFragmentShader", "shaders/ui.frag.spv");
    auto vertexShader = mShaderManager->GetShaderModule("UIVertexShader");
    auto fragmentShader = mShaderManager->GetShaderModule("UIFragmentShader");
    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = {vk::PipelineShaderStageCreateInfo()
                                                                         .setStage(vk::ShaderStageFlagBits::eVertex)
                                                                         .setModule(vertexShader)
                                                                         .setPName("main"),
                                                                     vk::PipelineShaderStageCreateInfo()
                                                                         .setStage(vk::ShaderStageFlagBits::eFragment)
                                                                         .setModule(fragmentShader)
                                                                         .setPName("main")};
    mConfig.setStages(shaderStages)
        .setLayout(mPipelineLayoutManager->GetPipelineLayout(PipelineLayoutType::UILayout))
        .setRenderPass(mRenderPassManager->GetRenderPass(RenderPassType::UI))
        .setSubpass(0);
    auto pipeline = context.GetDevice().createGraphicsPipelineUnique(nullptr, mConfig);
    if (pipeline.result != vk::Result::eSuccess)
    {
        LogE("Failed to create UI pipeline");
    }
    mPipelines[PipelineType::UI] = std::move(pipeline.value);
    LogD("Create UI pipeline success");
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
        LogE("Pipeline not found: {}", static_cast<int>(type));
        return nullptr;
    }
}
} // namespace MEngine