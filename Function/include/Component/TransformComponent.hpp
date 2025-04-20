#pragma once
#include "Buffer.hpp"
#include "CommandBuffeManager.hpp"
#include "Component/Interface/IComponent.hpp"
#include "Context.hpp"
#include "MEngine.hpp"
#include "entt/entity/storage.hpp"
#include "glm/fwd.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include <cstdint>
#include <memory>

namespace MEngine
{
struct TransformComponent : public IComponent<>
{
    glm::vec3 position{0.0f, 0.0f, 0.0f};
    glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 scale{1.0f, 1.0f, 1.0f};
    glm::mat4x4 modelMatrix;
    TransformComponent() = default;
    TransformComponent(glm::vec3 position, glm::quat rotation, glm::vec3 scale)
        : position(position), rotation(rotation), scale(scale)
    {
    }
};
} // namespace MEngine

// template <> struct entt::storage_type<MEngine::TransformComponent>
// {
//     using type = entt::storage<MEngine::TransformComponent>;
// };