#pragma once

#include "Component/Interface/IComponent.hpp"
#include "Entity/Interface/ILight.hpp"
#include "Entity/Light.hpp"
namespace MEngine
{
struct LightComponent : public IComponent<>
{
    float range = 0;     // 范围
    float coneAngle = 0; // 锥角 only for spot light

    glm::vec3 color = {1.0f, 1.0f, 1.0f}; // 颜色
    float intensity = 1.0f;               // 强度

    LightType type = LightType::Directional;
};
} // namespace MEngine