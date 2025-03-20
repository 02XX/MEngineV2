#pragma once

#include "Logger.hpp"
#include "MEngine.hpp"
#include "glm/glm.hpp"
#include <vulkan/vulkan.hpp>
namespace MEngine
{
class MENGINE_API Vertex
{
  public:
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    static std::array<vk::VertexInputAttributeDescription, 3> GetVertexInputAttributeDescription();
    static vk::VertexInputBindingDescription GetVertexInputBindingDescription();
};
} // namespace MEngine