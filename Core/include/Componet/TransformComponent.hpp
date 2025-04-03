#pragma once
#include "Buffer.hpp"
#include "CommandBuffeManager.hpp"
#include "Context.hpp"
#include "Logger.hpp"
#include "MEngine.hpp"

#include "glm/glm.hpp"
#include <cstdint>
#include <memory>
namespace MEngine
{
struct TransformComponent
{
    glm::vec3 mPosition;
    glm::vec3 mRotation;
    glm::vec3 mScale;
};
} // namespace MEngine