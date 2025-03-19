#pragma once
#include "Context.hpp"
#include "Logger.hpp"
#include "MEngine.hpp"
#include "SharedHandle.hpp"
#include <vulkan/vulkan.hpp>
namespace MEngine
{
struct GraphicsPipelineConfig
{
    // ===== 核心必需 =====
    vk::PipelineLayout pipelineLayout;
    vk::RenderPass renderPass;
    uint32_t subPass = 0;

    // ===== 着色器阶段 =====
    vk::ShaderModule vertexShader;
    vk::ShaderModule fragmentShader;

    // ===== 顶点输入 =====
    std::vector<vk::VertexInputBindingDescription> vertexBindings;
    std::vector<vk::VertexInputAttributeDescription> vertexAttributes;

    // ===== 光栅化 =====
    vk::PrimitiveTopology topology = vk::PrimitiveTopology::eTriangleList;
    vk::PolygonMode polygonMode = vk::PolygonMode::eFill;
    float lineWidth = 1.0f;
    vk::CullModeFlags cullMode = vk::CullModeFlagBits::eBack;
    vk::FrontFace frontFace = vk::FrontFace::eCounterClockwise;

    vk::Viewport viewport;
    vk::Rect2D scissor;

    // ===== 多重采样 =====
    vk::SampleCountFlagBits rasterizationSamples = vk::SampleCountFlagBits::e1;
    float minSampleShading = 1.0f;
    vk::Bool32 sampleShadingEnable = vk::False;
    vk::Bool32 alphaToCoverageEnable = vk::False;
    vk::Bool32 alphaToOneEnable = vk::False;
    std::vector<vk::SampleMask> sampleMasks = {0xFFFFFFFF}; // 默认全采样

    // ===== 颜色混合 =====
    std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachments;
    vk::Bool32 logicOpEnable = vk::False;
    vk::LogicOp logicOp = vk::LogicOp::eCopy;
    std::array<float, 4> blendConstants = {0.0f, 0.0f, 0.0f, 0.0f};
    // ===== 深度/模板 =====
    vk::Bool32 depthTestEnable = VK_TRUE;
    vk::Bool32 depthWriteEnable = VK_TRUE;
    vk::CompareOp depthCompareOp = vk::CompareOp::eLess;

    // ===== 深度边界测试 =====
    vk::Bool32 depthBoundsTestEnable = vk::False;
    float minDepthBounds = 0.0f;
    float maxDepthBounds = 1.0f;

    // ===== 模板测试 =====
    vk::Bool32 stencilTestEnable = vk::False;
    vk::StencilOpState frontStencilOp = vk::StencilOpState()
                                            .setFailOp(vk::StencilOp::eKeep)
                                            .setPassOp(vk::StencilOp::eKeep)
                                            .setDepthFailOp(vk::StencilOp::eKeep)
                                            .setCompareOp(vk::CompareOp::eNever);
    vk::StencilOpState backStencilOp = frontStencilOp;
};
struct ComputePipelineConfig
{
    uint32_t maxThreadsX = 256;
    uint32_t maxThreadsY = 1;
    uint32_t maxThreadsZ = 1;
};
using UniquePipeline = vk::UniquePipeline;
class MENGINE_API PipelineManager final
{
  public:
    PipelineManager() = default;
    /**
     * @brief Create a Graphics Pipeline object
     *
     * @param config
     * @return vk::UniquePipeline
     */
    UniquePipeline CreateUniqueGraphicsPipeline(const GraphicsPipelineConfig &config);
    SharedPipeline CreateSharedGraphicsPipeline(const GraphicsPipelineConfig &config);
    UniquePipeline CreateComputePipeline(const ComputePipelineConfig &config);
};
} // namespace MEngine
