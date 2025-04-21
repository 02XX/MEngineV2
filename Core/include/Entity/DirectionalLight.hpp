#pragma once

#include "Entity/Light.hpp"
namespace MEngine
{
class DirectionalLight : public Light
{
  public:
    LightType GetLightType() override
    {
        return LightType::Directional;
    }
    void SetLightType(LightType type) override
    {
        if (type != LightType::Directional)
        {
            throw std::invalid_argument("DirectionalLight can only be of type Directional.");
        }
    }
    glm::vec3 GetColor() override
    {
        return color;
    }
    void SetColor(const glm::vec3 &color) override
    {
        this->color = color;
    }
    float GetIntensity() override
    {
        return intensity;
    }
    void SetIntensity(float intensity) override
    {
        this->intensity = intensity;
    }

  public:
    DirectionalLight() : Light(LightType::Directional)
    {
    }
    DirectionalLight(const glm::vec3 &color, float intensity) : Light(LightType::Directional, color, intensity)
    {
    }
};
} // namespace MEngine