#pragma once
#include "Buffer.hpp"
#include "CommandBuffeManager.hpp"
#include "Context.hpp"
#include "MEngine.hpp"
#include "glm/fwd.hpp"
#include "glm/glm.hpp"
#include <cstdint>
#include <memory>
namespace MEngine
{
struct TransformComponent
{
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
};
} // namespace MEngine