#pragma once
#include "Math.hpp"
namespace MEngine
{
enum class LightType
{
    Directional = 0,
    Point = 1,
    Spot = 2,
    Area = 3,
};
struct Light
{
    LightType type;

    glm::vec3 position; // 位置
    float range;        // 范围

    glm::vec3 direction; // 方向
    float coneAngle;     // 锥角 only for spot light

    glm::vec3 color; // 颜色
    float intensity; // 强度
};
} // namespace MEngine