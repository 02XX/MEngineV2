#pragma once
#include "Buffer.hpp"
#include "MEngine.hpp"
#include "Mesh.hpp"
namespace MEngine
{
struct MeshComponent
{
    std::shared_ptr<Mesh> mesh;
};
} // namespace MEngine