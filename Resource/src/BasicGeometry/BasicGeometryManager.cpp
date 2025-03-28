#include "BasicGeometry/BasicGeometryManager.hpp"
namespace MEngine
{
std::shared_ptr<Mesh> BasicGeometryManager::GetPrimitive(PrimitiveType type)
{
    if (mCache.find(type) != mCache.end())
    {
        if (auto mesh = mCache[type].lock())
        {
            return mesh;
        }
    }
    std::shared_ptr<Mesh> mesh;
    switch (type)
    {
    case PrimitiveType::Cube:
        mesh = CreateCube();
        break;
    case PrimitiveType::Cylinder:
        mesh = CreateCylinder();
        break;
    case PrimitiveType::Sphere:
        mesh = CreateSphere();
        break;
    case PrimitiveType::Quad:
        mesh = CreateQuad();
        break;
    default:
        LogE("Primitive type not supported");
        break;
    }
    mCache[type] = mesh;
    return mesh;
}
std::shared_ptr<Mesh> BasicGeometryManager::CreateCube()
{
    // 立方体顶点数据（每个面4个顶点，共24个顶点）
    const std::vector<Vertex> vertices = {
        // 前面 (Z+)
        {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}, // 0
        {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},  // 1
        {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},   // 2
        {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},  // 3

        // 后面 (Z-)
        {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},  // 4
        {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}}, // 5
        {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},  // 6
        {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},   // 7

        // 左面 (X-)
        {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 8
        {{-0.5f, -0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},  // 9
        {{-0.5f, 0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},   // 10
        {{-0.5f, 0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},  // 11

        // 右面 (X+)
        {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},  // 12
        {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}, // 13
        {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},  // 14
        {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},   // 15

        // 顶面 (Y+)
        {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},  // 16
        {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},   // 17
        {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},  // 18
        {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}, // 19

        // 底面 (Y-)
        {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}}, // 20
        {{0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},  // 21
        {{0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},   // 22
        {{-0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},  // 23
    };
    // 索引数据（每个面2个三角形，共36个索引）
    const std::vector<uint32_t> indices = {// 前面 (Z+) 逆时针：0→1→2→3
                                           0, 1, 2, 0, 2, 3,
                                           // 后面 (Z-) 逆时针：4→5→6→7
                                           4, 5, 6, 4, 6, 7,
                                           // 左面 (X-) 逆时针：8→9→10→11
                                           8, 9, 10, 8, 10, 11,
                                           // 右面 (X+) 逆时针：12→13→14→15
                                           12, 13, 14, 12, 14, 15,
                                           // 顶面 (Y+) 逆时针：16→17→18→19
                                           16, 17, 18, 16, 18, 19,
                                           // 底面 (Y-) 逆时针：20→21→22→23
                                           20, 21, 22, 20, 22, 23};
    return std::make_shared<Mesh>(vertices, indices);
}
std::shared_ptr<Mesh> BasicGeometryManager::CreateCylinder()
{
    const int segments = 32;
    const float radius = 0.5f;
    const float height = 1.0f;
    const float halfHeight = height * 0.5f;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    // 顶部和底部中心点
    vertices.emplace_back(Vertex{{0, halfHeight, 0}, {0, 1, 0}, {0.5f, 0.5f}});   // Top center (0)
    vertices.emplace_back(Vertex{{0, -halfHeight, 0}, {0, -1, 0}, {0.5f, 0.5f}}); // Bottom center (1)

    // 生成环形顶点
    for (int i = 0; i <= segments; ++i)
    {
        float theta = 2.0f * glm::pi<float>() * i / segments;
        float x = radius * cos(theta);
        float z = radius * sin(theta);
        float u = static_cast<float>(i) / segments;

        // 顶部环形顶点
        vertices.emplace_back(Vertex{{x, halfHeight, z}, {0, 1, 0}, {u, 1.0f}}); // Top ring (2 + 2i)
        // 底部环形顶点
        vertices.emplace_back(Vertex{{x, -halfHeight, z}, {0, -1, 0}, {u, 0.0f}}); // Bottom ring (3 + 2i)
        // 侧面顶点
        vertices.emplace_back(Vertex{{x, halfHeight, z}, {x, 0, z}, {u, 1.0f}});  // Side top (4 + 2i)
        vertices.emplace_back(Vertex{{x, -halfHeight, z}, {x, 0, z}, {u, 0.0f}}); // Side bottom (5 + 2i)
    }

    // 生成索引
    for (int i = 0; i < segments; ++i)
    {
        // 顶部圆面（从外向内看为逆时针）
        indices.insert(indices.end(), {0u, static_cast<uint32_t>(2 + 2 * (i + 1)), static_cast<uint32_t>(2 + 2 * i)});

        // 底部圆面（从外向内看为逆时针）
        indices.insert(indices.end(), {1u, static_cast<uint32_t>(3 + 2 * i), static_cast<uint32_t>(3 + 2 * (i + 1))});

        // 侧面（外表面逆时针）
        uint32_t top = 4 + 2 * i;
        uint32_t nextTop = 4 + 2 * (i + 1);
        uint32_t bottom = 5 + 2 * i;
        uint32_t nextBottom = 5 + 2 * (i + 1);
        indices.insert(indices.end(), {top, bottom, nextTop});
        indices.insert(indices.end(), {nextTop, bottom, nextBottom});
    }
    return std::make_shared<Mesh>(vertices, indices);
}
std::shared_ptr<Mesh> BasicGeometryManager::CreateSphere()
{
    const int segments = 32;
    const int rings = 16;
    const float radius = 0.5f;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    // 生成顶点
    for (int y = 0; y <= rings; ++y)
    {
        float phi = glm::pi<float>() * y / rings;
        float yPos = radius * cos(phi);
        float ringRadius = radius * sin(phi);
        for (int x = 0; x <= segments; ++x)
        {
            float theta = 2.0f * glm::pi<float>() * x / segments;
            float xPos = ringRadius * cos(theta);
            float zPos = ringRadius * sin(theta);

            glm::vec3 normal = glm::normalize(glm::vec3(xPos, yPos, zPos));
            glm::vec2 uv(static_cast<float>(x) / segments, static_cast<float>(y) / rings);

            vertices.emplace_back(Vertex{{xPos, yPos, zPos}, normal, uv});
        }
    }

    // 生成索引
    for (int y = 0; y < rings; ++y)
    {
        for (int x = 0; x < segments; ++x)
        {
            uint32_t current = y * (segments + 1) + x;
            uint32_t next = current + segments + 1;

            // 逆时针顺序
            indices.insert(indices.end(), {current, current + 1, next});
            indices.insert(indices.end(), {current + 1, next + 1, next});
        }
    }
    return std::make_shared<Mesh>(vertices, indices);
}
std::shared_ptr<Mesh> BasicGeometryManager::CreateQuad()
{
    const std::vector<Vertex> vertices = {
        {{-0.5f, 0.0f, -0.5f}, {0, 1, 0}, {0, 0}}, // 0
        {{0.5f, 0.0f, -0.5f}, {0, 1, 0}, {1, 0}},  // 1
        {{0.5f, 0.0f, 0.5f}, {0, 1, 0}, {1, 1}},   // 2
        {{-0.5f, 0.0f, 0.5f}, {0, 1, 0}, {0, 1}},  // 3
    };

    const std::vector<uint32_t> indices = {0, 2, 1, 0, 3, 2};

    return std::make_shared<Mesh>(vertices, indices);
}
} // namespace MEngine