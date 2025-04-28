#pragma once
#include "Buffer.hpp"
#include "BufferFactory.hpp"
#include "MEngine.hpp"
#include "Vertex.hpp"

namespace MEngine
{
class Mesh
{
  private:
    std::vector<Vertex> mVertices;
    std::vector<uint32_t> mIndices;
};
} // namespace MEngine