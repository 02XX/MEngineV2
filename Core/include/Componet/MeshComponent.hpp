#pragma once
#include "Buffer.hpp"
#include "CommandBuffeManager.hpp"
#include "Context.hpp"
#include "Logger.hpp"
#include "MEngine.hpp"
#include "SharedHandle.hpp"
#include "glm/glm.hpp"
#include <cstdint>
#include <memory>
namespace MEngine
{
struct MENGINE_API MeshHandle
{
    uint32_t id;      // 唯一标识符
    std::string name; // 调试用名称
};
// 包围盒（AABB）
struct MENGINE_API BoundingBox
{
    glm::vec3 min{FLT_MAX};
    glm::vec3 max{-FLT_MAX};
};

// LOD 数据
struct MENGINE_API LODData
{
    uint32_t indexOffset = 0;     // 索引偏移量
    uint32_t indexCount = 0;      // 当前LOD的索引数量
    float screenSizeRatio = 1.0f; // 切换LOD的屏幕占比阈值
};
struct MENGINE_API MeshComponent
{
    // ===== 核心数据 =====
    MeshHandle handle; // 关联资源管理器的网格句柄

    // ===== 本地变换缓存 =====
    bool isBoundsDirty = true; // 包围盒是否需要重新计算
    BoundingBox localBounds;   // 模型空间包围盒（用于视锥剔除）

    // ===== 顶点/索引缓冲 =====
    UniqueBuffer vertexBuffer; // Vulkan 顶点缓冲区（由资源管理器填充）
    UniqueBuffer indexBuffer;  // Vulkan 索引缓冲区
    uint32_t indexCount = 0;   // 索引数量

    // ===== LOD 支持 =====
    uint32_t currentLOD = 0;   // 当前选中的 LOD 级别
    std::vector<LODData> LODs; // 多级细节数据（可选）

    // ===== 调试标记 =====
    bool isVisible = true;      // 是否参与渲染
    bool wireframeMode = false; // 线框模式覆盖（优先级高于材质）
};
} // namespace MEngine