#pragma once
#include "BufferManager.hpp"
#include "Componet/MaterialComponent.hpp"
#include "Componet/MeshComponent.hpp"
#include "Componet/TransformComponent.hpp"
#include "Logger.hpp"
#include "MEngine.hpp"
#include "PipelineLayoutManager.hpp"
#include "PipelineManager.hpp"
namespace MEngine
{
class BasicGeometry
{
  public:
    virtual ~BasicGeometry() = default;
    uint32_t ID;
    TransformComponent transform;
    MeshComponent mesh;
    MaterialComponent material;
};
} // namespace MEngine