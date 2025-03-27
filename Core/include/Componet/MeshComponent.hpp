#pragma once
#include "MEngine.hpp"
#include "Mesh.hpp"
namespace MEngine
{
struct MENGINE_API MeshComponent
{
    std::shared_ptr<Mesh> mesh;
};
} // namespace MEngine