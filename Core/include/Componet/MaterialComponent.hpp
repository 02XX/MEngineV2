#pragma once
#include "Buffer.hpp"
#include "Context.hpp"
#include "DescriptorManager.hpp"
#include "Logger.hpp"
#include "MEngine.hpp"
#include "SharedHandle.hpp"
#include "glm/glm.hpp"
#include <cstdint>
namespace MEngine
{
struct MENGINE_API MaterialHandle
{
    uint32_t id;      // 唯一标识符
    std::string name; // 调试用名称
};
struct MENGINE_API TextureHandle
{
    uint32_t id;      // 唯一标识符
    std::string name; // 调试用名称
};
struct MENGINE_API PipelineHandle
{
    uint32_t id;      // 唯一标识符
    std::string name; // 调试用名称
};
struct MENGINE_API DescriptorSetHandle
{
    uint32_t id;      // 唯一标识符
    std::string name; // 调试用名称
};
struct MENGINE_API PipelineLayoutHandle
{
    uint32_t id;      // 唯一标识符
    std::string name; // 调试用名称
};
struct MENGINE_API MaterialComponent
{
    // ===== 核心资源引用 =====
    MaterialHandle handle;   // 关联资源管理器的材质句柄
    PipelineHandle pipeline; // 当前使用的渲染管线（如 Forward/Deferred）

    // ===== 材质属性 =====
    glm::vec4 baseColor{1.0f};      // 基础颜色（RGBA）
    float metallic = 0.0f;          // 金属度
    float roughness = 0.5f;         // 粗糙度
    float emissiveIntensity = 0.0f; // 自发光强度

    // ===== 纹理绑定 =====
    TextureHandle albedoTexture;            // 反照率贴图
    TextureHandle normalTexture;            // 法线贴图
    TextureHandle metallicRoughnessTexture; // 金属粗糙度贴图（RG通道）

    // ===== 渲染状态覆盖 =====
    bool blendEnable = false; // 是否启用混合（覆盖管线默认）
    bool doubleSided = false; // 是否双面渲染（禁用背面剔除）

    // ===== Vulkan 特有 =====
    DescriptorSetHandle descriptorSet;   // 材质专属描述符集（包含UBO和纹理）
    PipelineLayoutHandle pipelineLayout; // 关联的管线布局
};
} // namespace MEngine