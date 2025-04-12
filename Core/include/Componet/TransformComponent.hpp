#pragma once
#include "Buffer.hpp"
#include "CommandBuffeManager.hpp"
#include "Context.hpp"
#include "MEngine.hpp"
#include "glm/fwd.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include <cstdint>
#include <memory>
namespace MEngine
{
struct TransformComponent
{
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;
    bool isNeedUpdate = true;
    glm::mat4x4 modelMatrix;
};
} // namespace MEngine